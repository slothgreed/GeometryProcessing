import { randomUUID } from 'node:crypto';
import { performance } from 'node:perf_hooks';
import { buildContext } from '../Context/context.mjs';
import { createTools } from '../Tools/tools.mjs';
import { createTrace } from '../Trace/trace.mjs';
import { withRetry } from '../Retry/retry.mjs';
import { demoAgent } from './demo-agent.mjs';

export async function run(request, directory, agent = demoAgent) {
  const runId = randomUUID();
  const trace = createTrace(directory, runId);
  const start = performance.now();
  trace('run_start', { caseId: request.id ?? null, agent: agent === demoAgent ? 'demo' : 'custom' });
  let result;
  try {
    const context = buildContext(request);
    trace('context', { context });
    // Call budget covers all attempts, not each retry separately.
    const tools = createTools(request.allowedTools ?? ['point_cloud_summary'], trace);
    const output = await withRetry(() => agent.run(context, tools), trace, request.maxAttempts ?? 2);
    result = { runId, ok: true, output };
  } catch (error) {
    result = { runId, ok: false, error: error.message };
  }
  result.durationMs = Math.round((performance.now() - start) * 100) / 100;
  trace('run_end', result);
  return result;
}
