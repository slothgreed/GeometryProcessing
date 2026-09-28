---
name: pointcloudapp-primitive
description: PointCloudAppのPrimitive、Mesh、Polylineなどの基本形状と生成処理を変更し、頂点表現、インデックス、更新通知、描画側との整合性を確認する。
---

# Primitiveの開発

対象はリポジトリ直下の `PointCloudApp/Primitive/`（このSkillからは `../../../PointCloudApp/Primitive/`）。本文中のコードパスは、リンク先を除きこの対象フォルダを基準にする。

共通の構成・ビルド手順は [モジュールのSkill](../pointcloudapp-development/SKILL.md) を参照する。この領域は基本形状の表現と生成を扱う。

## 調査の入口

- [Primitive.h](../../../PointCloudApp/Primitive/Primitive.h) と [Primitive.cpp](../../../PointCloudApp/Primitive/Primitive.cpp)：頂点属性、インデックス、描画種別、格納形式、更新フラグ。
- `Mesh`、`Polyline`：対象形状固有の表現と操作。
- `Primitives`：既存の形状生成処理。追加する形状に近い実装から呼び出し元まで確認する。
- `PrimitiveTable`は現状空のクラスなので、利用可能な形状管理機構として前提にしない。

## データの扱い

`Primitive`には属性別配列の`Array`と、`Vertex`配列の`Interleave`がある。現在の`Vertex`は位置と法線を持つ。`Convert`がすべての属性を一括変換すると仮定せず、色やUVを含めて利用側と照合する。

頂点数、必要な属性の要素数、インデックスの範囲、描画種別を確認する。三角形数や線分数は、インデックスの有無とプリミティブ種別に依存するので、配列長だけから決めない。

setterを呼べば更新フラグが自動的に立つとは限らない。`Update`、`NeedUpdate`、`ClearUpdate`の利用箇所を追い、形状変更が描画側へ届くことを確認する。境界ボックスや法線の更新も別途確認する。

現在の`CalcNormal`は位置ベクトルを正規化する簡易処理であり、一般的なメッシュの面から法線を求める処理として扱わない。`Multi`で座標を変換する際も、法線や境界まで同時に更新されると仮定しない。

## 検証

小さな形状で頂点数、属性数、添字、巻き順、有限性を確認する。格納形式の変換を変更した場合は往復変換と属性の保持を、生成処理なら端点や閉じ方を確認する。GPUレイアウトへ影響する変更はバッファ作成とシェーダー入力まで追い、必要な表示確認を行う。
