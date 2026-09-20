import { readFileSync, writeFileSync, mkdirSync } from 'node:fs';
import { fileURLToPath } from 'node:url';
import { join } from 'node:path';
import { randomUUID } from 'node:crypto';
import { run } from './Runner/runner.mjs';
import { evaluate } from './Evals/evaluate.mjs';

const root = fileURLToPath(new URL('.', import.meta.url));
const args = process.argv.slice(2);
try {
  if (!(args.length === 1 && args[0] === '--all') &&
      !(args.length === 2 && args[0] === '--case')) {
    throw new Error('Usage: node HarnessAgent/cli.mjs --all | --case <request.json>');
  }
  const cases = args[0] === '--all'
    ? JSON.parse(readFileSync(join(root, 'Evals/cases.json'), 'utf8'))
    : [JSON.parse(readFileSync(args[1], 'utf8'))];
  if (!Array.isArray(cases) || cases.length === 0 || cases.some(c => !c || typeof c !== 'object')) {
    throw new Error('cases must contain request objects');
  }
  const directory = join(root, 'Runs', randomUUID());
  mkdirSync(directory, { recursive: true });
  const results = [];
  for (const request of cases) {
    const result = await run(request, directory);
    const evaluation = evaluate(result, request.expected);
    results.push({ caseId: request.id ?? null, ...result, ...evaluation });
    console.log(`[${evaluation.passed ? 'PASS' : 'FAIL'}] ${request.id ?? result.runId}: ${evaluation.reason}`);
  }
  writeFileSync(join(directory, 'summary.json'), JSON.stringify({ agent: 'demo', results }, null, 2), 'utf8');
  const passed = results.filter(r => r.passed).length;
  console.log(`Passed: ${passed}/${results.length}\nResults: ${directory}`);
  process.exitCode = passed === results.length ? 0 : 1;
} catch (error) {
  console.error(error.message);
  process.exitCode = 2;
}
