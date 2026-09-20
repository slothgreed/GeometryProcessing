# HarnessAgent

PointCloudAppでAgent Harnessを試すための小さな実行基盤です。
「入力 → コンテキスト → Agent → Tool → トレース → 期待値との比較」を実行できます。
Node.js 18以降の標準機能だけを使うため、npm installやAPIキーは不要です。

## 最初に実行する

リポジトリのルートで実行します。

```powershell
node HarnessAgent/cli.mjs --all
node --test HarnessAgent/Evals/harness.test.mjs
```

最初のコマンドはJSONの5ケースを実行します。正常な3ケースと、空入力・Tool未許可の
拒否を確認する2ケースです。拒否ケースは「期待どおり拒否された」場合にPASSです。
この成功率はハーネスの動作確認であり、LLMの能力評価ではありません。

結果は毎回 `Runs/<batch UUID>/` に保存します。

- `summary.json`: ケースごとの結果、評価理由、所要時間、実行ID。
- `<run UUID>.jsonl`: コンテキスト、試行、Tool引数・結果・エラー、再試行、終了結果。

終了コードは全評価PASSが0、評価FAILが1、CLI引数・入力ファイルなどのエラーが2です。
ログはGit管理対象外ですが、入力座標とプロンプトをそのまま含みます。

## ファイルの役割

| ファイル | 役割 |
| --- | --- |
| `cli.mjs` | ケース一括実行と集計 |
| `Runner/runner.mjs` | 実行ID、コンテキスト、Tool、再試行の組み合わせ |
| `Runner/demo-agent.mjs` | SDK接続箇所を示す決定的なデモAgent |
| `Context/context.mjs` | 明示された入力だけをコンテキスト化 |
| `Tools/tools.mjs` | 点群の点数・境界ボックス、許可リスト、呼び出し上限 |
| `Trace/trace.mjs` | JSONLのイベント保存 |
| `Retry/retry.mjs` | 一時的エラーのみ上限付きで再試行 |
| `Evals/evaluate.mjs` | 成功結果または拒否理由を期待値と比較 |
| `Evals/cases.json` | 編集して増やせる評価ケース |
| `Evals/harness.test.mjs` | リトライ、Tool制御、失敗判定の回帰テスト |

## ケースを追加する

`Evals/cases.json`にオブジェクトを追加するか、同じ形式のオブジェクト1個を別のJSONに保存します。

```json
{
  "id": "my-points",
  "prompt": "点群の点数と範囲を調べてください。",
  "points": [[0, 0, 0], [10, 20, 30]],
  "allowedTools": ["point_cloud_summary"],
  "maxAttempts": 2,
  "expected": {
    "ok": true,
    "output": { "count": 2, "min": [0, 0, 0], "max": [10, 20, 30] }
  }
}
```

```powershell
node HarnessAgent/cli.mjs --case HarnessAgent/my-case.json
```

`points`は有限な数値のXYZ配列です。単位は入力に従い、mmなどは仮定しません。
現在の評価は値の完全一致です。幾何演算を追加した場合は演算に合う許容誤差や
トポロジーの評価を追加してください。期待値はAgentへ渡しません。

## SDKへ接続するとき

差し替え先は `async run(context, tools)` を持つオブジェクトです。
`Runner/runner.mjs` の `run(request, directory, agent)` の第3引数へ渡します。
CLIで使う場合は `cli.mjs` のrun呼び出しでそのAgentを指定し、集計のagent表記も更新します。
SDK用のToolスキーマを定義し、呼び出しは必ず `tools.call(name, args)` に通します。
使用可能なToolの情報は `tools.definitions` にあります。

再試行するのは `RetryableError` だけで、既定2回、最大3回です。
再試行はAgent全体を再実行します。現在のToolは読み取り専用です。
将来、形状を変更するToolを追加するときは、二重実行を防ぐ処理を用意してから
再試行を有効にしてください。Tool呼び出しは再試行を含めて最大4回です。

## 実装済みと今後の接続

実装済み: ローカル実行、Toolによる実際の点数・境界計算、許可制御、呼び出し上限、
ログ、限定的な再試行、JSON評価と集計。

未接続: LLM/Agent SDK、自然言語の解釈、PointCloudAppのC++処理、GeometryAIの名前付きパイプ、
CAD生成、トークン・費用計測、SDK通信のタイムアウトとキャンセル。
デモAgentはプロンプトを解釈せず、常に点群集計Toolを呼びます。
SDK接続時はそのSDKのキャンセル機構を使って時間制限も追加してください。

C++アプリとは独立しており、ソリューションへの登録やMSBuildは不要です。
既存の `HalfEdgeHarness.md` に記載された幾何検証CLIは、次の段階でToolとして
接続する候補です。ここではその実行やアセット変更は行いません。
