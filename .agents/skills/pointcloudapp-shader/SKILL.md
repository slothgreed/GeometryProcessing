---
name: pointcloudapp-shader
description: PointCloudAppのGLSLシェーダーを調査・変更し、C++側で組み立てるソース、GPUデータレイアウト、描画・Computeの入出力を照合して検証する。
---

# Shaderの開発

対象はリポジトリ直下の `PointCloudApp/Shader/`（このSkillからは `../../../PointCloudApp/Shader/`）。本文中のコードパスは、リンク先を除きこの対象フォルダを基準にする。

共通の構成・ビルド手順は [モジュールのSkill](../pointcloudapp-development/SKILL.md) を参照する。ここは実行時に読み込むシェーダーソースであり、C++側のシェーダークラスは`../Renderer/Shader/`などにある。

## ソースの読み方

対象ファイル名をC++側から検索し、`GetShaderPath`と [IShader.cpp](../../../PointCloudApp/Renderer/Shader/IShader.cpp) の組み立て処理を確認する。読み込みはversion、define、extension、共通ヘッダー、ステージ本文を組み合わせるため、ファイル単体がコンパイル単位とは限らない。

[version.h](../../../PointCloudApp/Shader/version.h) は現在GLSL 460を指定する。[common.h](../../../PointCloudApp/Shader/common.h) はカメラ、ライト、クラスタ、間接描画などの共有定義を持つ。ComputeやMesh系ではC++側がlocal sizeなどを挿入する経路もある。本文へ重複した宣言を加えない。

## 変更時の照合

- ステージ間の入出力、頂点属性のlocation、uniform名、binding、textureの型をC++側と合わせる。
- バッファ構造体はフィールド順序、型と符号、サイズ、アライメント、配列長、適用されるレイアウト規則を確認する。共有データは`../GL/RenderResource.h`の`ShaderLayout`など転送元まで追う。
- Computeでは処理対象の要素数、dispatchのグループ数、local size、端数の範囲外アクセスを確認する。書き込み後の利用先に応じた同期はC++側の呼び出しまで調べる。
- 深度、座標空間、法線、色の計算を変える場合は、入力がどの空間・形式かを確認する。既存の変換やトーンマッピングを重複させない。
- 共通ヘッダーを変更するときは、そのヘッダーを組み込むシェーダーの参照を調べて影響範囲を決める。Mesh/Taskなど拡張依存のステージでは対象GPUと要求拡張も確認する。

## 検証

アプリが組み立てたソースで実行時のコンパイルとリンクを確認する。エラー位置はファイル単体の行番号と一致しない場合があるため、挿入されたヘッダーや定義も含めて読む。C++ビルドの成功だけでGLSLの成功としない。

対象の描画・Compute処理を実行して出力を確認する。境界や要素数を変える修正では、ちょうど割り切れる入力だけでなく端数のある入力も確認する。実行時確認ができなければ、ソース上の照合と未確認のGPU動作を分けて報告する。
