import { isDeepStrictEqual } from 'node:util';

export function evaluate(result, expected) {
  if (!expected || typeof expected.ok !== 'boolean') {
    return { passed: false, reason: 'expected.ok is required' };
  }
  if (result.ok !== expected.ok) return { passed: false, reason: 'unexpected success/failure' };
  if (expected.ok) {
    const passed = Object.hasOwn(expected, 'output') && isDeepStrictEqual(result.output, expected.output);
    return { passed, reason: passed ? 'output matched' : 'output mismatch' };
  }
  const passed = typeof expected.errorContains === 'string' && expected.errorContains.length > 0
    && result.error.includes(expected.errorContains);
  return { passed, reason: passed ? 'expected rejection' : 'error mismatch' };
}
