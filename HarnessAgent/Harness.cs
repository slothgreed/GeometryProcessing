using System.Diagnostics;
using System.Text.Json.Nodes;

namespace HarnessAgent;

public interface IAgent
{
    string Name
    {
        get;
    }
    string? Model => null;
    Task<JsonNode?> RunAsync(AgentContext context, ToolRegistry tools, RunTrace trace, CancellationToken cancellationToken);
}

public sealed class DemoAgent : IAgent
{
    public string Name => "demo";
    public Task<JsonNode?> RunAsync(AgentContext context, ToolRegistry tools, RunTrace trace, CancellationToken cancellationToken)
    {
        cancellationToken.ThrowIfCancellationRequested();
        return Task.FromResult<JsonNode?>(tools.Call(ToolRegistry.SummaryTool, new JsonObject { ["points"] = context.Points?.DeepClone() }));
    }
}

public static class Harness
{
    public static async Task<JsonObject> RunAsync(JsonObject request, string directory, IAgent agent, CancellationToken cancellationToken = default)
    {
        var id = Guid.NewGuid().ToString();
        var trace = new RunTrace(directory, id);
        var timer = Stopwatch.StartNew();
        trace.Write("run_start", new()
        {
            ["caseId"] = request["id"]?.DeepClone(),
            ["agent"] = agent.Name,
            ["model"] = agent.Model
        });
        JsonObject result;
        try
        {
            var context = AgentContext.Build(request);
            trace.Write("context", new()
            {
                ["prompt"] = context.Prompt,
                ["points"] = context.Points?.DeepClone()
            });
            if (request.ContainsKey("allowedTools") && request["allowedTools"] is not JsonArray)
            {
                throw new ToolPolicyException("allowedTools must be an array");
            }

            var allowed = request["allowedTools"] is JsonArray names
                ? names.Select(n => n!.GetValue<string>()).ToArray() : [ToolRegistry.SummaryTool];
            var tools = new ToolRegistry(allowed, trace); // Budget is shared by all retry attempts.
            var output = await RetryAsync(() => agent.RunAsync(context, tools, trace, cancellationToken),
                trace, request["maxAttempts"]?.GetValue<int>() ?? 2, cancellationToken);
            result = new()
            {
                ["ok"] = true,
                ["output"] = output
            };
        }
        catch (OperationCanceledException) when (cancellationToken.IsCancellationRequested)
        {
            trace.Write("cancelled");
            throw;
        }
        catch (Exception error)
        {
            result = new() { ["ok"] = false, ["error"] = error.Message };
        }
        result["runId"] = id;
        result["durationMs"] = timer.Elapsed.TotalMilliseconds;
        trace.Write("run_end", result);
        return result;
    }

    public static (bool Passed, string Reason) Evaluate(JsonObject result, JsonObject? expected)
    {
        if (expected?["ok"] is not JsonValue flag || !flag.TryGetValue<bool>(out var expectedOk))
        {
            return (false, "expected.ok is required");
        }

        if (result["ok"]!.GetValue<bool>() != expectedOk)
        {
            return (false, "unexpected success/failure");
        }

        if (expectedOk)
        {
            var passed = expected.ContainsKey("output") && JsonNode.DeepEquals(result["output"], expected["output"]);
            return (passed, passed ? "output matched" : "output mismatch");
        }
        var text = expected["errorContains"]?.GetValue<string>();
        var matched = !string.IsNullOrEmpty(text) && (result["error"]?.GetValue<string>()?.Contains(text, StringComparison.Ordinal) ?? false);
        return (matched, matched ? "expected rejection" : "error mismatch");
    }

    private static async Task<T> RetryAsync<T>(Func<Task<T>> action, RunTrace trace,
        int maxAttempts, CancellationToken cancellationToken)
    {
        if (maxAttempts is < 1 or > 3)
        {
            throw new ArgumentException("maxAttempts must be 1..3");
        }

        for (var attempt = 1; ; attempt++)
        {
            cancellationToken.ThrowIfCancellationRequested();
            trace.Write("attempt", new()
            {
                ["attempt"] = attempt
            });
            try
            {
                return await action();
            }
            catch (RetryableException error) when (attempt < maxAttempts)
            {
                trace.Write("retry", new()
                {
                    ["attempt"] = attempt,
                    ["error"] = error.Message
                });
                await Task.Delay(100 * attempt, cancellationToken);
            }
        }
    }
}

public sealed record AgentContext(string Instruction, string Prompt, JsonNode? Points)
{
    public static AgentContext Build(JsonObject request)
    {
        var prompt = request["prompt"]?.GetValue<string>();
        if (string.IsNullOrWhiteSpace(prompt))
        {
            throw new ArgumentException("prompt must be a non-empty string");
        }
        // Expected answers stay in the evaluator, never in the model's context.
        return new("Analyze supplied points using allowed tools. Do not infer physical units.",
            prompt, request["points"]?.DeepClone());
    }
}

public sealed class RunTrace(string directory, string runId)
{
    public void Write(string name, JsonObject? data = null)
    {
        Directory.CreateDirectory(directory);
        var entry = new JsonObject
        {
            ["time"] = DateTimeOffset.UtcNow.ToString("O"),
            ["runId"] = runId,
            ["event"] = name,
            ["data"] = data?.DeepClone()
        };
        File.AppendAllText(Path.Combine(directory, runId + ".jsonl"), entry.ToJsonString() + Environment.NewLine);
    }
}

public sealed class RetryableException(string message) : Exception(message);

