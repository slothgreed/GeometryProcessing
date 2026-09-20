export const definitions = {
  point_cloud_summary: {
    description: 'Return point count and axis-aligned bounds; units match the input.',
    readOnly: true,
    input: '{ points: number[3][] } (non-empty, finite coordinates)',
  },
};

function summarize({ points } = {}) {
  if (!Array.isArray(points) || points.length === 0 || points.some(
    p => !Array.isArray(p) || p.length !== 3 || p.some(x => !Number.isFinite(x)))) {
    throw new Error('points must be a non-empty array of finite XYZ triples');
  }
  const min = [...points[0]], max = [...points[0]];
  for (const p of points) {
    for (let axis = 0; axis < 3; axis++) {
      min[axis] = Math.min(min[axis], p[axis]);
      max[axis] = Math.max(max[axis], p[axis]);
    }
  }
  return { count: points.length, min, max };
}

export function createTools(allowed, trace, maxCalls = 4) {
  if (!Array.isArray(allowed) || allowed.some(name => !Object.hasOwn(definitions, name))) {
    throw new Error('allowedTools contains an unknown tool');
  }
  let calls = 0;
  return {
    definitions: Object.fromEntries(allowed.map(name => [name, definitions[name]])),
    async call(name, args) {
      trace('tool_call', { name, args });
      try {
        if (++calls > maxCalls) throw new Error('tool call limit exceeded');
        if (!allowed.includes(name)) throw new Error(`tool not allowed: ${name}`);
        const result = summarize(args);
        trace('tool_result', { name, result });
        return result;
      } catch (error) {
        trace('tool_error', { name, error: error.message });
        throw error;
      }
    },
  };
}
