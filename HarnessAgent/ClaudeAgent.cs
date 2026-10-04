using System.Net.Http.Json;
using System.Text.Json.Nodes;

namespace HarnessAgent;

// The Messages API protocol is intentionally visible here for learning.
public sealed class ClaudeAgent : IAgent
{
    private readonly HttpClient client;
    private readonly string apiKey;
    private readonly TimeSpan timeout;
    private readonly int maxTurns;
    public string Name => "claude";
    public string Model
    {
        get;
    }

    public ClaudeAgent(HttpClient client, string? apiKey, string? model, int maxTurns = 6, TimeSpan? timeout = null)
    {
        if (string.IsNullOrWhiteSpace(apiKey))
        {
            throw new ArgumentException("ANTHROPIC_API_KEY is required");
        }

        if (string.IsNullOrWhiteSpace(model))
        {
            throw new ArgumentException("ANTHROPIC_MODEL is required");
        }

        if (maxTurns is < 1 or > 10)
        {
            throw new ArgumentException("maxTurns must be 1..10");
        }

        this.timeout = timeout ?? TimeSpan.FromSeconds(30);
        if (this.timeout <= TimeSpan.Zero || this.timeout > TimeSpan.FromMinutes(2))
        {
            throw new ArgumentException("invalid timeout");
        }

        this.client = client;
        this.apiKey = apiKey;
        Model = model;
        this.maxTurns = maxTurns;
    }

    private async Task<JsonObject> SendAsync(JsonObject body, RunTrace trace, CancellationToken cancellationToken)
    {
        using var deadline = CancellationTokenSource.CreateLinkedTokenSource(cancellationToken);
        deadline.CancelAfter(timeout);
        using var request = new HttpRequestMessage(HttpMethod.Post, "https://api.anthropic.com/v1/messages");
        request.Headers.Add("x-api-key", apiKey);
        request.Headers.Add("anthropic-version", "2023-06-01");
        request.Content = JsonContent.Create(body);
        try
        {
            using var response = await client.SendAsync(request, HttpCompletionOption.ResponseHeadersRead, deadline.Token);
            if (!response.IsSuccessStatusCode)
            {
                var status = (int)response.StatusCode;
                trace.Write("api_error", new()
                {
                    ["status"] = status
                });
                // Never include headers, key, or raw server error body in logs.
                if (status == 429 || status >= 500)
                {
                    throw new RetryableException($"Claude API HTTP {status}");
                }

                throw new InvalidOperationException($"Claude API HTTP {status}");
            }
            var bodyText = await response.Content.ReadAsStringAsync(deadline.Token);
            var data = JsonNode.Parse(bodyText) as JsonObject ?? throw new InvalidOperationException("invalid API response");
            trace.Write("api_response", new()
            {
                ["model"] = data["model"]?.DeepClone(),
                ["stopReason"] = data["stop_reason"]?.DeepClone(),
                ["usage"] = data["usage"]?.DeepClone()
            });
            return data;
        }
        catch (OperationCanceledException) when (!cancellationToken.IsCancellationRequested)
        {
            throw new RetryableException("Claude API timeout");
        }
        catch (HttpRequestException)
        {
            throw new RetryableException("Claude API network failure");
        }
    }

    public async Task<JsonNode?> RunAsync(AgentContext context, ToolRegistry tools, RunTrace trace, CancellationToken cancellationToken)
    {
        if (tools.Definitions.Count == 0)
        {
            throw new ToolPolicyException("tool not allowed: no tools enabled");
        }

        var messages = new JsonArray(new JsonObject
        {
            ["role"] = "user",
            ["content"] = new JsonObject
            {
                ["prompt"] = context.Prompt,
                ["points"] = context.Points?.DeepClone()
            }.ToJsonString()
        });
        var succeeded = false;
        ToolInputException? lastError = null;
        for (var turn = 1; turn <= maxTurns; turn++)
        {
            trace.Write("api_request", new()
            {
                ["turn"] = turn,
                ["model"] = Model
            });
            var response = await SendAsync(new()
            {
                ["model"] = Model,
                ["max_tokens"] = 1024,
                ["system"] = context.Instruction + " Use tools to obtain the answer. Preserve the original points exactly. Never invent or repair input data. After a successful tool call, return only the result JSON with count, min, max and no markdown. If tool arguments fail, correct them to match the original input or explain the failure.",
                ["tools"] = tools.Definitions,
                ["messages"] = messages.DeepClone()
            }, trace, cancellationToken);
            var content = response["content"] as JsonArray ?? throw new InvalidOperationException("API response has no content");
            messages.Add(new JsonObject { ["role"] = "assistant", ["content"] = content.DeepClone() });
            var stop = response["stop_reason"]?.GetValue<string>();
            if (stop == "tool_use")
            {
                var results = new JsonArray();
                foreach (var call in content.OfType<JsonObject>().Where(b => b["type"]?.GetValue<string>() == "tool_use"))
                {
                    var id = call["id"]?.GetValue<string>() ?? throw new InvalidOperationException("missing tool id");
                    var name = call["name"]?.GetValue<string>() ?? throw new InvalidOperationException("missing tool name");
                    try
                    {
                        var output = tools.Call(name, call["input"]);
                        succeeded = true;
                        lastError = null;
                        results.Add(new JsonObject { ["type"] = "tool_result", ["tool_use_id"] = id, ["content"] = output.ToJsonString() });
                    }
                    catch (ToolInputException error)
                    {
                        lastError = error;
                        results.Add(new JsonObject { ["type"] = "tool_result", ["tool_use_id"] = id, ["is_error"] = true, ["content"] = error.Message });
                    }
                }
                if (results.Count == 0)
                {
                    throw new InvalidOperationException("tool_use response contains no calls");
                }

                messages.Add(new JsonObject { ["role"] = "user", ["content"] = results });
                continue;
            }
            if (stop != "end_turn")
            {
                throw new InvalidOperationException($"Claude stopped: {stop}");
            }

            if (lastError is not null)
            {
                throw lastError;
            }

            if (!succeeded)
            {
                throw new InvalidOperationException("Claude finished without a successful tool call");
            }

            var answer = string.Concat(content.OfType<JsonObject>().Where(b => b["type"]?.GetValue<string>() == "text")
                .Select(b => b["text"]?.GetValue<string>()));
            trace.Write("agent_answer", new()
            {
                ["answer"] = answer
            });
            try
            {
                return JsonNode.Parse(answer) as JsonObject ?? throw new InvalidOperationException("Claude final answer must be a JSON object");
            }
            catch (System.Text.Json.JsonException)
            {
                throw new InvalidOperationException("Claude final answer is not valid JSON");
            }
        }
        throw new InvalidOperationException("Claude turn limit exceeded");
    }
}
