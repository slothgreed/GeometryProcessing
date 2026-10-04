---
name: pointcloudapp-development
description: PointCloudAppモジュールの機能追加、不具合修正、コード調査を行う際に、アプリ制御、幾何処理、シーンノード、OpenGL描画の関係を把握し、既存の実験環境で実装・検証する。
---

# PointCloudAppの開発

幾何処理とリアルタイムグラフィックスを試すWindows向けC++の実験用アプリケーション。既存機能を利用し、依頼された実験が動き、結果を確かめられる状態にする。

モジュールルートはリポジトリ直下の `PointCloudApp/`（このSkillからは `../../../PointCloudApp/`）。本文中のコードパスは、リンク先を除きモジュールルートを基準にする。共通の開発方針は [AGENTS.md](../../../AGENTS.md) を参照する。本Skillはプロジェクトを扱うための初版であり、個別のアルゴリズムやデータ形式の詳細は対象コードで確認する。

## 作業の入口

- 依頼が形状処理、表示、操作UI、入出力のどこに関わるかを整理する。関連する既存機能から読み始め、必要な呼び出し元と依存先へ調査を広げる。
- [main.cpp](../../../PointCloudApp/main.cpp) で実際に起動するアプリとコマンドライン分岐を確認する。実験用アプリが複数あるので、クラス名だけで実行対象を決めない。
- 入力データ、操作、期待する出力を対応付ける。既存のサンプルや検証用分岐が目的に合えば利用するが、固定パスと出力先の条件を先に確認する。

## 構成と役割

| 場所 | 主な役割・調査の入口 |
| --- | --- |
| `App/` | アプリ制御。`GLFWApp`はウィンドウ・入力の基盤、`PointCloudApp`はシーン構築、実行ループ、UI。別の実験用アプリも含む。 |
| `Algorithm/` | KDTree、BVH、Delaunay、Voxelなどの幾何処理。HalfEdgeの構造・ローダー・ノードは`Algorithm/HalfEdge/`にある。 |
| `Primitive/` | `Primitive`、`Mesh`、`Polyline`などの形状表現。CPUデータとGPUデータの関係は各実装で確認する。 |
| `Node/` | `RenderNode`を基点にしたシーン構造。点群、GLTF、STEPなどの機能とローダー。 |
| `GL/` | `GLContext`による描画状態操作、`RenderResource`による共有リソース、`RenderPass`、`RenderTarget`、GPUバッファ。 |
| `Renderer/` | PBR、ライトのカリング、カメラ、C++側のシェーダー制御。 |
| `Shader/` | 実行時に読み込むシェーダーコード。C++側の呼び出しと組にして調べる。 |
| `Utility/` | ファイル操作、数学補助、入力情報、計測、外部プロセスとの通信。 |

[pch.h](../../../PointCloudApp/pch.h) には`KI`で使用する型別名と共通includeがある。x64設定では強制includeされるため、ファイル単体で型定義が見つからない場合はここを確認する。

## 処理のつながり

`main.cpp`は選択したアプリの`Initialize`、`Execute`、`Finalize`を呼ぶ。メインアプリの実行ループは [App/PointCloudApp.cpp](../../../PointCloudApp/App/PointCloudApp.cpp) を確認する。シーン生成用の`Create...`関数とその呼び出し箇所を追い、どのデータが表示されるかを把握する。

[Node/RenderNode.h](../../../PointCloudApp/Node/RenderNode.h) は描画、更新、UI、ピッキングの接点。派生ノードでは`DrawNode`、`UpdateData`、`ShowUI`などの拡張点を確認する。`DrawContext`は描画リソースとパスを渡し、`UIContext`と`PickContext`は操作側の情報を渡す。描画パスマスクもあるため、描画処理の追加時は対象パスを確認する。

[GL/RenderResource.h](../../../PointCloudApp/GL/RenderResource.h) はカメラ、ライト、シェーダー、描画先などをまとめる。描画の問題では、ノードの入力データ、GPUへの転送、描画パス、シェーダー、描画先のつながりを追う。

## 変更内容ごとの進め方

### 幾何処理・データ形式

既存の形状表現とアルゴリズムの入出力を確認してから変更する。頂点・面・HalfEdgeの添字、境界の表現、座標変換を呼び出し元と照合する。ローダーではファイル形式と読み込み後の内部表現を区別する。

小さく決定的な入力で、要素数、境界、トポロジー、数値の有限性を確認する。表示が必要な場合は既存ノードへ結果を渡し、計算と描画の問題を切り分ける。

### 描画・シェーダー

対象ノード、`Renderer/Shader/`のC++処理、対応する`Shader/`のコードを組にして確認する。`ShaderTable`を使う機能では生成・取得経路も追う。

GPUバッファ変更時は、C++構造体とシェーダー側のフィールド順序、サイズ、アライメント、bindingを照合する。共有レイアウトには`RenderResource.h`の`ShaderLayout`がある。OpenGLリソースの寿命、状態変更と復帰、描画先のサイズ変更への対応を既存の呼び出し順に合わせる。

### UI・入力・シーンへの組み込み

アプリ全体のUIは`PointCloudApp::ShowUI`、対象ごとのUIはノードの`ShowUI`を入口にする。入力は`GLFWApp`、カメラ操作、ピッキング、対象ノードまでを必要な範囲で追う。追加機能は、既存のシーン生成箇所やUIから実際に呼ばれるところまで接続する。

### GeometryAIとの連携

[Utility/ProcessExecutor.h](../../../PointCloudApp/Utility/ProcessExecutor.h) の`AIProcessor`と`ClientPipe`が入口。AI処理本体は別実行ファイルのGeometryAIにある。通信変更時は送信するコマンド名・引数順を受信側と照合する。PointCloudApp側だけの依頼からモデルや学習処理の変更へ範囲を広げない。

## 実装とビルド

- includeはヘッダーのパス名を基準に、大文字・小文字を区別せずABC順に並べる。連続したincludeのまとまりごとに整列し、条件付きコンパイル、マクロ定義、意味のあるコメント・グループ境界を越えて移動しない。GLEWをOpenGL・GLFWより先に読むなど、順序依存がある場合は必要な順序を優先し、理由をコメントで残す。

- 周囲の実装と現在のフィルターに合わせて配置する。ファイル追加時は [PointCloudApp.vcxproj](../../../PointCloudApp/PointCloudApp.vcxproj) と [PointCloudApp.vcxproj.filters](../../../PointCloudApp/PointCloudApp.vcxproj.filters) の両方へ登録する。
- モジュール内のincludeはルート基準のパスを使う。例：`#include "GL/Buffer/GLBuffer.h"`。
- include整理で未定義の型が出た場合は、必要なヘッダーを利用側が直接includeしているか確認する。単なるinclude漏れは直接の依存を追加して修正し、並び順の例外とは区別する。
- 実行時データやシェーダーの読み込みにはローカルパスの前提がある。起動・読み込みの問題では、作業ディレクトリと実際の参照先を確認する。
- Visual Studio 2022、MSVC v143、C++20、x64を基本に、影響するPointCloudAppプロジェクトをビルドする。単体ビルドでは`SolutionDir`にリポジトリルートを末尾区切り付きで渡す。依存設定は構成で異なるため、Releaseの成功からDebugの成功を推測しない。

## 完了時の確認

実装した処理と使い方、検証に使った入力や操作、ビルド構成と結果を報告する。雛形や未接続の処理は実装済みの機能と区別する。描画変更では実行時のシェーダーコンパイル・リンクと表示を確認し、確認できなかった範囲を明記する。
