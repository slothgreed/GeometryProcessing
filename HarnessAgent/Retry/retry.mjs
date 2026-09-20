// Adapters may use this only for transient failures of operations safe to repeat.
export class RetryableError extends Error {}

export async function withRetry(action, trace, maxAttempts = 2) {
  if (!Number.isInteger(maxAttempts) || maxAttempts < 1 || maxAttempts > 3) {
    throw new Error('maxAttempts must be an integer between 1 and 3');
  }
  for (let attempt = 1; attempt <= maxAttempts; attempt++) {
    trace('attempt', { attempt });
    try {
      return await action();
    } catch (error) {
      if (!(error instanceof RetryableError) || attempt === maxAttempts) throw error;
      trace('retry', { attempt, error: error.message });
      await new Promise(resolve => setTimeout(resolve, 100 * attempt));
    }
  }
}
