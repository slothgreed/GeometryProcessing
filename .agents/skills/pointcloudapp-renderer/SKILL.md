---
name: pointcloudapp-renderer
description: PointCloudAppのPBR、ライトのカリング、カメラ、C++側のシェーダー制御を変更し、描画方式とシーン・GPUリソースの接続を確認する。
---

# Rendererの開発

対象はリポジトリ直下の `PointCloudApp/Renderer/`（このSkillからは `../../../PointCloudApp/Renderer/`）。本文中のコードパスは、リンク先を除きこの対象フォルダを基準にする。

共通の構成・ビルド手順は [モジュールのSkill](../pointcloudapp-development/SKILL.md) を参照する。この領域は描画方式とそのC++側の制御を扱う。描画リソース基盤は`../GL/`、シェーダー本文は`../Shader/`にある。

## 調査の入口

- `PBR`と`Light`：材質・照明のデータと利用側。
- `ClusteredLightCuller`：クラスタ分割、ライトの登録・選別、関連するCompute処理と描画側の参照。
- `Camera/`：視点・投影・カメラ操作。
- [Shader/IShader.h](../../../PointCloudApp/Renderer/Shader/IShader.h)：シェーダーパス、ステージ、uniform、バッファ、描画・Computeの基盤。
- [Shader/ShaderTable.h](../../../PointCloudApp/Renderer/Shader/ShaderTable.h)：共有シェーダーの生成と取得。機能によって個別にシェーダーを持つ経路もあるので、実際の所有者を確認する。

## 変更時のつながり

呼び出すノードやアプリ、使用する`RenderResource`、C++シェーダークラス、シェーダー本文の順に接続を確認する。追加したシェーダーは、生成・パラメーター設定・実行までつなぐ。

カメラ変更はview、projection、それらの積と逆行列、near/far、画面サイズ、フラスタムを利用する経路に影響する。表示だけでなく、関係するピッキングやカリングも確認する。

ライトのカリング変更では、クラスタ数、深度の区分、最大ライト数、GPUバッファ容量とシェーダー側の添字計算を照合する。定数やレイアウトは`../GL/RenderResource.h`や`../Shader/common.h`にも関係するため、片側だけで判断しない。

描画設定の変更では、深度パス・通常描画・後処理の順序と入力を確認する。C++側の頂点属性番号、uniform、texture unit、SSBOのbindingはシェーダー側と組にして変更する。

## 検証

x64ビルドと実行時のシェーダーコンパイル・リンクを確認する。変更した機能に応じて、カメラ移動、画面リサイズ、ライトの範囲や個数を変えて結果を確認する。性能を評価する場合はCPU時間とGPU時間を区別し、入力条件を揃える。表示未確認の範囲は明記する。
