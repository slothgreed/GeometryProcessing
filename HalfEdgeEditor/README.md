# HalfEdge Editor

Three.jsを使ったHalf-edgeメッシュ編集環境のWebフロントエンドです。

現在は、`PointCloudApp/HalfEdgeLoader.cpp`と互換性のあるversion 2の`.half`バイナリを読み込み、Stanford Bunnyを表示します。

## Development

```powershell
npm install
npm run dev
```

ブラウザで `http://localhost:3000` を開きます。

## 面積による再分割

左パネルの「再分割」で目標面積（上限）と最大分割回数を指定します。
初期目標は読み込み時の平均面積です。最大の面から最長辺を中点で二分割し、
共有する隣接面も同じ頂点で分割します。1回は1辺の分割で、内部辺では2面増加します。
上限に達した場合は残りの対象面数を表示し、再実行で続行できます。

既存頂点と巻き順を維持し、描画用インデックスからHalf-edgeの接続と代表辺を再構築します。
境界のoppositeEdgeは-1です。不正な辺の共有・向き、縮退面、座標の精度限界では変更を適用しません。
小さい面の縮約や、完全な面積均一化は行いません。
画面上の面積ではなく、モデル空間での面積を基準にします。

### 再分割後の辺の改善

再分割後は、対角の和が180°を超える内部辺を別の対角線に入れ替えます。
頂点座標と頂点・辺・面の数を維持し、2面の最小角が改善する場合だけ採用します。
境界辺と処理開始時に法線の差が30°以上ある辺は固定します。
途中で30°以上の折れ目となった辺、重複辺、反転・縮退する面も処理しません。
新しい面の法線は元の両面の法線から30°以内に制限します。
入れ替え後の面積は、目標面積と対象2面の面積の最大値を超えないようにします。
最大10巡で停止し、入れ替え回数と反復上限への到達を表示します。
分割が不要な場合も辺の改善を実行します。

これは3Dの直線辺を入れ替える局所改善で、元の曲面を厳密に保つ内在的ドロネー三角形分割ではありません。
曲面上では表面形状と総面積が変化し得ます。保護条件によりドロネー条件を満たさない辺も残ります。
離れた面との交差判定は含みません。
参考: [Geometry Centralの内在的三角形分割](https://geometry-central.net/surface/intrinsic_triangulations/basics/)。

検証: `node --test tests/remeshing.test.mjs`（境界、共有辺、閉曲面、辺の入れ替え、折れ目保護、実データ、入力検証）。

## Binary layout

- Header: `version`, `vertexCount`, `halfEdgeCount`, `faceCount` (`int32`)
- Vertex: `x`, `y`, `z` (`float32`) and representative edge (`int32`)
- Half-edge: `endPos`, `nextEdge`, `beforeEdge`, `oppositeEdge`, `face` (`int32`)
- Face: representative edge (`int32`)

数値はlittle-endianです。座標はC++版に合わせて `(z, y, x)` へ変換します。
