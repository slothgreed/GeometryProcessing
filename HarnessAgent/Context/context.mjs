// Only explicit task data is passed to the agent. Repository files are not auto-loaded.
export function buildContext(request) {
  if (typeof request.prompt !== 'string' || !request.prompt.trim()) {
    throw new Error('prompt must be a non-empty string');
  }
  return {
    instruction: 'Analyze the supplied points using allowed tools. Do not infer physical units.',
    prompt: request.prompt,
    points: structuredClone(request.points),
  };
}
