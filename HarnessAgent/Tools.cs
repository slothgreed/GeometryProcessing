using System.Text.Json.Nodes;

namespace HarnessAgent;

public sealed class ToolPolicyException(string message) : Exception(message);
public sealed class ToolInputException(string message) : Exception(message);

public sealed class ToolRegistry
{
    public const string SummaryTool = "point_cloud_summary";
    private readonly HashSet<string> allowed;
    private readonly RunTrace trace;
    private int calls;

    public ToolRegistry(IEnumerable<string> allowedTools, RunTrace trace)
    {
        allowed = allowedTools.ToHashSet(StringComparer.Ordinal);
        if (allowed.Any(name => name != SummaryTool))
        {
            throw new ToolPolicyException("unknown allowed tool");
        }

        this.trace = trace;
    }

    public JsonArray Definitions => allowed.Contains(SummaryTool)
        ? new JsonArray(JsonNode.Parse("""
          {"name":"point_cloud_summary","description":"Return point count and axis-aligned bounds. Units match input.",
           "input_schema":{"type":"object","properties":{"points":{"type":"array","minItems":1,
           "items":{"type":"array","items":{"type":"number"},"minItems":3,"maxItems":3}}},
           "required":["points"],"additionalProperties":false}}
          """)) : new JsonArray();

    public JsonObject Call(string name, JsonNode? args)
    {
        trace.Write("tool_call", new()
        {
            ["name"] = name,
            ["args"] = args?.DeepClone()
        });
        try
        {
            if (++calls > 4)
            {
                throw new ToolPolicyException("tool call limit exceeded");
            }

            if (!allowed.Contains(name))
            {
                throw new ToolPolicyException($"tool not allowed: {name}");
            }

            var result = Summarize(args);
            trace.Write("tool_result", new()
            {
                ["name"] = name,
                ["result"] = result.DeepClone()
            });
            return result;
        }
        catch (Exception error)
        {
            trace.Write("tool_error", new()
            {
                ["name"] = name,
                ["error"] = error.Message
            });
            throw;
        }
    }

    private static JsonObject Summarize(JsonNode? args)
    {
        if (args is not JsonObject obj || obj.Count != 1 || obj["points"] is not JsonArray { Count: > 0 } points)
        {
            throw new ToolInputException("points must be a non-empty array of finite XYZ triples");
        }

        var min = new[] { double.PositiveInfinity, double.PositiveInfinity, double.PositiveInfinity };
        var max = new[] { double.NegativeInfinity, double.NegativeInfinity, double.NegativeInfinity };
        foreach (var point in points)
        {
            if (point is not JsonArray { Count: 3 } xyz)
            {
                throw new ToolInputException("points must contain finite XYZ triples");
            }

            for (var axis = 0; axis < 3; axis++)
            {
                if (xyz[axis] is not JsonValue value || !value.TryGetValue<double>(out var number) || !double.IsFinite(number))
                {
                    throw new ToolInputException("points must contain finite XYZ triples");
                }

                min[axis] = Math.Min(min[axis], number);
                max[axis] = Math.Max(max[axis], number);
            }
        }
        return new()
        {
            ["count"] = points.Count,
            ["min"] = new JsonArray(min.Select(x => (JsonNode?)JsonValue.Create(x)).ToArray()),
            ["max"] = new JsonArray(max.Select(x => (JsonNode?)JsonValue.Create(x)).ToArray())
        };
    }
}
