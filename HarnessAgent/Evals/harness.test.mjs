import test from 'node:test';
import assert from 'node:assert/strict';
import { mkdtempSync, readFileSync, rmSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { join } from 'node:path';
import { run } from '../Runner/runner.mjs';
import { RetryableError } from '../Retry/retry.mjs';
import { evaluate } from './evaluate.mjs';

const request = { prompt: 'Summarize points', points: [[0, 0, 0]] };
async function fixture(action) {
  const directory = mkdtempSync(join(tmpdir(), 'pointcloud-harness-'));
  try { await action(directory); }
  finally { rmSync(directory, { recursive: true }); }
}

test('transient failure retries once and retains the trace', () => fixture(async directory => {
  let attempts = 0;
  const result = await run(request, directory, {
    async run(context, tools) {
      if (++attempts === 1) throw new RetryableError('temporary failure');
      return tools.call('point_cloud_summary', { points: context.points });
    },
  });
  assert.equal(result.ok, true);
  assert.equal(attempts, 2);
  const events = readFileSync(join(directory, `${result.runId}.jsonl`), 'utf8')
    .trim().split('\n').map(line => JSON.parse(line).event);
  assert.deepEqual(events, ['run_start', 'context', 'attempt', 'retry', 'attempt', 'tool_call', 'tool_result', 'run_end']);
}));

test('persistent transient failure stops at the attempt limit', () => fixture(async directory => {
  let attempts = 0;
  const result = await run(request, directory, {
    async run() { attempts++; throw new RetryableError('still unavailable'); },
  });
  assert.equal(result.ok, false);
  assert.equal(attempts, 2);
}));

test('denied tools do not retry', () => fixture(async directory => {
  const result = await run({ ...request, allowedTools: [] }, directory);
  assert.equal(result.ok, false);
  assert.match(result.error, /tool not allowed/);
  const log = readFileSync(join(directory, `${result.runId}.jsonl`), 'utf8');
  assert.equal(log.includes('"event":"retry"'), false);
}));

test('tool budget also applies across retries', () => fixture(async directory => {
  const result = await run(request, directory, {
    async run(context, tools) {
      for (let i = 0; i < 3; i++) await tools.call('point_cloud_summary', { points: context.points });
      throw new RetryableError('try again');
    },
  });
  assert.equal(result.ok, false);
  assert.match(result.error, /tool call limit/);
}));

test('malformed and nonfinite coordinates are rejected', () => fixture(async directory => {
  for (const points of [[[1, 2]], [[0, Infinity, 1]], [[0, '1', 2]]]) {
    assert.equal((await run({ ...request, points }, directory)).ok, false);
  }
}));

test('incorrect expected geometry and absent criteria fail evaluation', () => {
  const result = { ok: true, output: { count: 1, min: [0, 0, 0], max: [0, 0, 0] } };
  assert.equal(evaluate(result, { ok: true, output: { ...result.output, count: 2 } }).passed, false);
  assert.equal(evaluate(result, undefined).passed, false);
});
