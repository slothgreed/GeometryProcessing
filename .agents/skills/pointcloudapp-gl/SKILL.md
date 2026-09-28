---
name: pointcloudapp-gl
description: PointCloudAppのOpenGL状態管理、GPUバッファ、描画先、共有描画リソース、描画パスを調査・変更し、生成から解放までの整合性を確認する。
---

# GLの開発

対象はリポジトリ直下の `PointCloudApp/GL/`（このSkillからは `../../../PointCloudApp/GL/`）。本文中のコードパスは、リンク先を除きこの対象フォルダを基準にする。

共通の構成・ビルド手順は [モジュールのSkill](../pointcloudapp-development/SKILL.md) を参照する。この領域はOpenGL操作と描画リソースの基盤を扱う。

## 調査の入口

- [GLContext.h](../../../PointCloudApp/GL/GLContext.h)：深度、ブレンド、カリング、ビューポート、描画先の切り替え。`GLStatus`のキャッシュと実際のOpenGL状態の関係も確認する。
- [RenderResource.h](../../../PointCloudApp/GL/RenderResource.h)：カメラ・ライトのGPUデータ、シェーダー、描画先などの共有リソース。`Build`、更新、`Finalize`の呼び出しを追う。
- [RenderPass.h](../../../PointCloudApp/GL/RenderPass.h)：`IRenderPass::Execute`と各パスの`Begin`、`End`。ノード側のパスマスクとの接続を確認する。
- `RenderTarget`と`Buffer/`：描画先、テクスチャ、フレームバッファ、バッファの作成・転送・解放。
- `GLAPIExt`：拡張機能の初期化と利用経路。

## 変更時に追うこと

リソースの所有者、作成時点、再利用・再作成条件、解放時点を確認する。GLコンテキストが有効な期間に操作されることと、ウィンドウサイズ変更後のサイズ・アタッチメントの整合性を確かめる。

`PushRenderTarget`と`PopRenderTarget`を使う経路では、描画先のスタックを対応させる。状態変更は既存の`GLContext`経路を優先して確認し、直接のGL呼び出しを加える場合はキャッシュとの食い違いを避ける。

GPUデータ変更時は、要素数とバイト数、型、stride、offset、bindingを転送側と利用側で照合する。`RenderResource.h`の`ShaderLayout`を変更するなら、シェーダー側の構造体も同時に確認する。

描画パス変更時は実行順序、色・深度の読み書き、クリア、ノードの参加条件を追う。描画先の追加だけで機能が完成したと判断しない。

## 検証

x64ビルドに加え、対象機能の実行、必要なフレームバッファの完全性確認、リサイズ、リソース再作成・終了時の動作を変更内容に応じて確認する。GPU処理の出力を後段で使う場合は同期と可視性の条件も確認する。実行時確認ができなければ、その範囲を明記する。
