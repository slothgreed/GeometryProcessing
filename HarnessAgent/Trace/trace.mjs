import { appendFileSync, mkdirSync } from 'node:fs';
import { join } from 'node:path';

export function createTrace(directory, runId) {
  mkdirSync(directory, { recursive: true });
  const path = join(directory, `${runId}.jsonl`);
  return (event, data = {}) => appendFileSync(path,
    JSON.stringify({ time: new Date().toISOString(), runId, event, ...data }) + '\n', 'utf8');
}
