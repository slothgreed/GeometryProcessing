using System.Net;
using System.Text;
using System.Text.Json.Nodes;

namespace HarnessAgent;

// Dependency-free offline regression tests. No requests reach Anthropic.
public static class SelfTests
{
    private static JsonObject Object(string json) => JsonNode.Parse(json)!.AsObject();
    private static JsonObject Request() => Object("""{"prompt":"範囲を調べて","points":[[1,2,3]]}""");
    private const string Output = """{"count":1,"min":[1,2,3],"max":[1,2,3]}""";
    private static string Call(string points = "[[1,2,3]]", string name = ToolRegistry.SummaryTool) =>
        new JsonObject
        {
            ["stop_reason"] = "tool_use",
            ["content"] = new JsonArray(new JsonObject
            {
                ["type"] = "tool_use",
                ["id"] = "t1",
                ["name"] = name,
                ["input"] = new JsonObject { ["points"] = JsonNode.Parse(points) }
            })
        }.ToJsonString();
    private static string Final(string output = Output, string stop = "end_turn") => new JsonObject
    {
        ["stop_reason"] = stop,
        ["usage"] = new JsonObject { ["input_tokens"] = 10, ["output_tokens"] = 20 },
        ["content"] = new JsonArray(new JsonObject { ["type"] = "text", ["text"] = output })
    }.ToJsonString();
    private static void Check(bool condition, string message = "assertion failed")
    {
        if (!condition)
        {
            throw new Exception(message);
        }
    }
    private static void Failed(JsonObject result, string text)
    {
        Check(result["ok"]!.GetValue<bool>() == false, "expected failure");
        Check(result["error"]!.GetValue<string>().Contains(text, StringComparison.Ordinal), result.ToJsonString());
    }

    private sealed class FakeHandler(params string[] responses) : HttpMessageHandler
    {
        public List<JsonObject> Requests { get; } = [];
        public int Status { get; init; } = 200;
        public bool WaitForCancellation
        {
            get; init;
        }
        protected override async Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
        {
            Check(request.RequestUri!.AbsoluteUri == "https://api.anthropic.com/v1/messages");
            Check(request.Headers.GetValues("anthropic-version").Single() == "2023-06-01");
            Requests.Add(Object(await request.Content!.ReadAsStringAsync(cancellationToken)));
            if (WaitForCancellation)
            {
                await Task.Delay(Timeout.Infinite, cancellationToken);
            }

            var index = Requests.Count - 1;
            if (Status == 200)
            {
                Check(index < responses.Length, "unexpected API call");
            }

            return new HttpResponseMessage((HttpStatusCode)Status)
            {
                Content = new StringContent(Status == 200 ? responses[index] : "secret-error-body", Encoding.UTF8, "application/json")
            };
        }
    }

    private sealed class TransientAgent(int failures) : IAgent
    {
        public int Attempts
        {
            get; private set;
        }
        public string Name => "test";
        public Task<JsonNode?> RunAsync(AgentContext context, ToolRegistry tools, RunTrace trace, CancellationToken cancellationToken)
        {
            if (++Attempts <= failures)
            {
                throw new RetryableException("temporary failure");
            }

            return new DemoAgent().RunAsync(context, tools, trace, cancellationToken);
        }
    }

    public static async Task<int> RunAsync()
    {
        var root = Path.Combine(Path.GetTempPath(), "HarnessAgent-tests-" + Guid.NewGuid());
        Directory.CreateDirectory(root);
        var tests = new List<(string Name, Func<string, Task> Run)>
        {
            ("context excludes expected answers", directory =>
            {
                var request = Request(); request["expected"] = Object(Output);
                var context = AgentContext.Build(request);
                request["points"]![0]![0] = 999;
                Check(context.Points![0]![0]!.GetValue<int>() == 1);
                return Task.CompletedTask;
            }),
            ("transient failure retries and succeeds", async directory =>
            {
                var agent = new TransientAgent(1);
                var result = await Harness.RunAsync(Request(), directory, agent);
                Check(result["ok"]!.GetValue<bool>() && agent.Attempts == 2);
            }),
            ("persistent failure stops after two attempts", async directory =>
            {
                var agent = new TransientAgent(9);
                Failed(await Harness.RunAsync(Request(), directory, agent), "temporary");
                Check(agent.Attempts == 2);
            }),
            ("denied tool does not retry", async directory =>
            {
                var request = Request(); request["allowedTools"] = new JsonArray();
                Failed(await Harness.RunAsync(request, directory, new DemoAgent()), "tool not allowed");
                Check(!File.ReadAllText(Directory.GetFiles(directory).Single()).Contains("\"event\":\"retry\""));
            }),
            ("invalid coordinates rejected", async directory =>
            {
                foreach (var input in new[] { "[]", "[[1,2]]", "[[1,\"2\",3]]", "[[1,null,3]]", "[[1,1e999,3]]" })
                {
                    var request = Request(); request["points"] = JsonNode.Parse(input);
                    Check(!(await Harness.RunAsync(request, directory, new DemoAgent()))["ok"]!.GetValue<bool>());
                }
            }),
            ("tool budget survives repeated calls", directory =>
            {
                var tools = new ToolRegistry([ToolRegistry.SummaryTool], new RunTrace(directory, "budget"));
                var input = Object("""{"points":[[1,2,3]]}""");
                for (var i = 0; i < 4; i++)
                {
                    tools.Call(ToolRegistry.SummaryTool, input);
                }
                try
                {
                    tools.Call(ToolRegistry.SummaryTool, input);
                    throw new Exception("limit not enforced");
                }
                catch (ToolPolicyException)
                {
                }
                return Task.CompletedTask;
            }),
            ("evaluation catches mismatches and missing criteria", directory =>
            {
                var result = new JsonObject { ["ok"] = true, ["output"] = Object(Output) };
                Check(!Harness.Evaluate(result, null).Passed);
                Check(!Harness.Evaluate(result, Object("""{"ok":true,"output":{}}""")).Passed);
                Check(Harness.Evaluate(result, new JsonObject { ["ok"] = true, ["output"] = Object(Output) }).Passed);
                return Task.CompletedTask;
            }),
            ("Claude round trip and usage trace", async directory =>
            {
                using var fake = new FakeHandler(Call(), Final()); using var client = new HttpClient(fake);
                var result = await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "test-key", "test-model"));
                Check(result["ok"]!.GetValue<bool>(), result.ToJsonString());
                Check(JsonNode.DeepEquals(result["output"], Object(Output)));
                Check(fake.Requests.Count == 2);
                Check(fake.Requests[1]["messages"]![2]!["content"]![0]!["tool_use_id"]!.GetValue<string>() == "t1");
                Check(!fake.Requests[0].ToJsonString().Contains("expected"));
                var log = File.ReadAllText(Directory.GetFiles(directory).Single());
                Check(log.Contains("output_tokens") && !log.Contains("test-key"));
            }),
            ("Claude receives tool errors and corrects arguments", async directory =>
            {
                using var fake = new FakeHandler(Call("[]"), Call(), Final()); using var client = new HttpClient(fake);
                Check((await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model")))["ok"]!.GetValue<bool>());
                Check(fake.Requests[1]["messages"]![2]!["content"]![0]!["is_error"]!.GetValue<bool>());
            }),
            ("unknown tools blocked", async directory =>
            {
                using var fake = new FakeHandler(Call(name: "shell")); using var client = new HttpClient(fake);
                Failed(await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model")), "tool not allowed");
                Check(fake.Requests.Count == 1);
            }),
            ("HTTP errors retry only when transient", async directory =>
            {
                foreach (var status in new[] { 401, 429, 500, 529 })
                {
                    using var fake = new FakeHandler { Status = status }; using var client = new HttpClient(fake);
                    Failed(await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model")), $"HTTP {status}");
                    Check(fake.Requests.Count == (status == 401 ? 1 : 2));
                }
            }),
            ("timeout terminates and retries within limit", async directory =>
            {
                using var fake = new FakeHandler { WaitForCancellation = true }; using var client = new HttpClient(fake);
                Failed(await Harness.RunAsync(Request(), directory,
                    new ClaudeAgent(client, "key", "model", timeout: TimeSpan.FromMilliseconds(20))), "timeout");
                Check(fake.Requests.Count == 2);
            }),
            ("caller cancellation does not retry", async directory =>
            {
                using var fake = new FakeHandler { WaitForCancellation = true }; using var client = new HttpClient(fake);
                using var cancel = new CancellationTokenSource(TimeSpan.FromMilliseconds(20));
                try
                {
                    await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model"), cancel.Token);
                    throw new Exception("not cancelled");
                }
                catch (OperationCanceledException)
                {
                    Check(fake.Requests.Count <= 1);
                }
            }),
            ("truncated or ungrounded responses fail", async directory =>
            {
                foreach (var stop in new[] { "max_tokens", "end_turn", "refusal" })
                {
                    using var fake = new FakeHandler(Final(stop: stop)); using var client = new HttpClient(fake);
                    Check(!(await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model")))["ok"]!.GetValue<bool>());
                }
                using var invalid = new FakeHandler(Call(), Final("invalid JSON")); using var invalidClient = new HttpClient(invalid);
                Failed(await Harness.RunAsync(Request(), directory, new ClaudeAgent(invalidClient, "key", "model")), "not valid JSON");
            }),
            ("turn limit enforced", async directory =>
            {
                using var fake = new FakeHandler(Call()); using var client = new HttpClient(fake);
                Failed(await Harness.RunAsync(Request(), directory, new ClaudeAgent(client, "key", "model", maxTurns: 1)), "turn limit");
            }),
            ("no tools means no network call", async directory =>
            {
                using var fake = new FakeHandler(); using var client = new HttpClient(fake);
                var request = Request(); request["allowedTools"] = new JsonArray();
                Failed(await Harness.RunAsync(request, directory, new ClaudeAgent(client, "key", "model")), "tool not allowed");
                Check(fake.Requests.Count == 0);
            }),
            ("missing API configuration rejected", directory =>
            {
                using var client = new HttpClient();
                foreach (var (key, model) in new[] { ("", "model"), ("key", "") })
                {
                    try
                    {
                        _ = new ClaudeAgent(client, key, model);
                        throw new Exception("missing config accepted");
                    }
                    catch (ArgumentException)
                    {
                    }
                }
                return Task.CompletedTask;
            })
        };
        var failed = 0;
        try
        {
            for (var i = 0; i < tests.Count; i++)
            {
                try
                {
                    await tests[i].Run(Path.Combine(root, i.ToString()));
                    Console.WriteLine($"[PASS] {tests[i].Name}");
                }
                catch (Exception error)
                {
                    failed++;
                    Console.WriteLine($"[FAIL] {tests[i].Name}: {error.Message}");
                }
            }
        }
        finally
        {
            Directory.Delete(root, recursive: true);
        }
        Console.WriteLine($"Tests: {tests.Count - failed}/{tests.Count}");
        return failed == 0 ? 0 : 1;
    }
}

