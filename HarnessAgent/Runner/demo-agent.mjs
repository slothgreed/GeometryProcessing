// Deterministic harness demo, not an LLM: the prompt is not interpreted.
// SDK adapter contract: async run(context, tools) -> JSON-serializable result.
// Route SDK tool requests through tools.call(name, args).
export const demoAgent = {
  async run(context, tools) {
    return tools.call('point_cloud_summary', { points: context.points });
  },
};
