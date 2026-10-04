global using System.Net.Http;
using System.Text.Json;
using System.Text.Json.Nodes;

namespace HarnessAgent;

internal static class Program
{
    [STAThread]
    private static void Main(string[] args)
    {
        if (args.Length == 0)
        {
            ApplicationConfiguration.Initialize();
            Application.Run(new MainForm());
            return;
        }

        Environment.ExitCode = RunCommandLineAsync(args).GetAwaiter().GetResult();
    }

    private static async Task<int> RunCommandLineAsync(string[] args)
    {
        using var cancellation = new CancellationTokenSource();
        Console.CancelKeyPress += (_, e) => { e.Cancel = true; cancellation.Cancel(); };
        try
        {
            if (args.SequenceEqual(new[] { "--self-test" }))
            {
                return await SelfTests.RunAsync();
            }

            var arguments = args.ToList();
            var provider = "claude";
            if (arguments.Count >= 2 && arguments[0] == "--agent")
            {
                provider = arguments[1];
                arguments.RemoveRange(0, 2);
            }
            if (provider is not ("claude" or "demo"))
            {
                throw new ArgumentException("agent must be claude or demo");
            }

            if (!(arguments.Count == 1 && arguments[0] == "--all") && !(arguments.Count == 2 && arguments[0] == "--case"))
            {
                throw new ArgumentException("Usage: HarnessAgent [--agent claude|demo] --all | --case <json> | --self-test");
            }

            var input = await File.ReadAllTextAsync(arguments[0] == "--all"
                ? Path.Combine(AppContext.BaseDirectory, "Evals", "cases.json") : arguments[1], cancellation.Token);
            JsonArray cases = arguments[0] == "--all"
                ? JsonNode.Parse(input) as JsonArray ?? throw new ArgumentException("cases must be an array")
                : new JsonArray(JsonNode.Parse(input));
            if (cases.Count == 0 || cases.Any(c => c is not JsonObject))
            {
                throw new ArgumentException("cases must contain request objects");
            }

            using var client = new HttpClient(new HttpClientHandler { AllowAutoRedirect = false }) { Timeout = Timeout.InfiniteTimeSpan };
            IAgent agent = provider == "demo" ? new DemoAgent()
                : new ClaudeAgent(client, Environment.GetEnvironmentVariable("ANTHROPIC_API_KEY"), Environment.GetEnvironmentVariable("ANTHROPIC_MODEL"));
            // Results stay alongside the executable, independent of the current directory.
            var directory = Path.Combine(AppContext.BaseDirectory, "Runs", Guid.NewGuid().ToString());
            Directory.CreateDirectory(directory);
            var results = new JsonArray();
            var passed = 0;
            foreach (var request in cases.OfType<JsonObject>())
            {
                var result = await Harness.RunAsync(request, directory, agent, cancellation.Token);
                var evaluation = Harness.Evaluate(result, request["expected"] as JsonObject);
                result["caseId"] = request["id"]?.DeepClone();
                result["passed"] = evaluation.Passed;
                result["reason"] = evaluation.Reason;
                results.Add(result);
                if (evaluation.Passed)
                {
                    passed++;
                }

                Console.WriteLine($"[{(evaluation.Passed ? "PASS" : "FAIL")}] {request["id"]}: {evaluation.Reason}");
                // Keep completed cases on disk even when a later case is cancelled.
                await File.WriteAllTextAsync(Path.Combine(directory, "summary.json"), new JsonObject
                {
                    ["agent"] = agent.Name,
                    ["model"] = agent.Model,
                    ["results"] = results.DeepClone()
                }
                    .ToJsonString(new JsonSerializerOptions { WriteIndented = true }), cancellation.Token);
            }
            Console.WriteLine($"Passed: {passed}/{cases.Count}\nResults: {directory}");
            return passed == cases.Count ? 0 : 1;
        }
        catch (OperationCanceledException)
        {
            Console.Error.WriteLine("Cancelled");
            return 130;
        }
        catch (Exception error)
        {
            Console.Error.WriteLine(error.Message);
            return 2;
        }
    }
}
