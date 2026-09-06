# HalfEdge Editor

Three.jsを使ったHalf-edgeメッシュ編集環境のWebフロントエンドです。

現在は、`PointCloudApp/HalfEdgeLoader.cpp`と互換性のあるversion 2の`.half`バイナリを読み込み、Stanford Bunnyを表示します。

## Development

```powershell
npm install
npm run dev
```

ブラウザで `http://localhost:3000` を開きます。

## Binary layout

- Header: `version`, `vertexCount`, `halfEdgeCount`, `faceCount` (`int32`)
- Vertex: `x`, `y`, `z` (`float32`) and representative edge (`int32`)
- Half-edge: `endPos`, `nextEdge`, `beforeEdge`, `oppositeEdge`, `face` (`int32`)
- Face: representative edge (`int32`)

数値はlittle-endianです。座標はC++版に合わせて `(z, y, x)` へ変換します。
