export type HalfEdgeMeshData = {
  version:number; vertexCount:number; halfEdgeCount:number; faceCount:number;
  positions:Float32Array; indices:Uint32Array; halfEdges:Int32Array;
  positionToEdge:Int32Array; faceToEdge:Int32Array;
};

const HEADER_BYTES=16;
const INDEXED_VERTEX_BYTES=16;
const HALF_EDGE_FIELDS=5;
const HALF_EDGE_BYTES=HALF_EDGE_FIELDS*Int32Array.BYTES_PER_ELEMENT;

export function parseHalfEdgeFile(buffer:ArrayBuffer):HalfEdgeMeshData {
  if(buffer.byteLength<HEADER_BYTES) throw new Error('ファイルが短すぎます。');
  const view=new DataView(buffer);
  const version=view.getInt32(0,true);
  const vertexCount=view.getInt32(4,true);
  const halfEdgeCount=view.getInt32(8,true);
  const faceCount=view.getInt32(12,true);
  if(version!==2) throw new Error(`未対応のHalf-edge形式です (version ${version})。`);
  if(vertexCount<=0||halfEdgeCount<=0||faceCount<=0) throw new Error('頂点・Half-edge・面の要素数が不正です。');
  const expectedBytes=HEADER_BYTES+vertexCount*INDEXED_VERTEX_BYTES+halfEdgeCount*HALF_EDGE_BYTES+faceCount*4;
  if(buffer.byteLength!==expectedBytes) throw new Error(`ファイルサイズが形式と一致しません (${buffer.byteLength} / ${expectedBytes} bytes)。`);

  let offset=HEADER_BYTES;
  const positions=new Float32Array(vertexCount*3);
  const positionToEdge=new Int32Array(vertexCount);
  for(let vertex=0;vertex<vertexCount;vertex+=1){
    const x=view.getFloat32(offset,true), y=view.getFloat32(offset+4,true), z=view.getFloat32(offset+8,true);
    // Match HalfEdgeLoader.cpp: Vector3(z, y, x).
    positions[vertex*3]=z; positions[vertex*3+1]=y; positions[vertex*3+2]=x;
    positionToEdge[vertex]=view.getInt32(offset+12,true);
    offset+=INDEXED_VERTEX_BYTES;
  }
  const halfEdges=new Int32Array(halfEdgeCount*HALF_EDGE_FIELDS);
  for(let i=0;i<halfEdges.length;i+=1){ halfEdges[i]=view.getInt32(offset,true); offset+=4; }
  const faceToEdge=new Int32Array(faceCount);
  const indices=new Uint32Array(faceCount*3);
  for(let face=0;face<faceCount;face+=1){
    const edgeIndex=view.getInt32(offset,true); offset+=4; faceToEdge[face]=edgeIndex;
    assertIndex(edgeIndex,halfEdgeCount,'faceToEdge');
    const beforeEdge=halfEdges[edgeIndex*HALF_EDGE_FIELDS+2];
    const nextEdge=halfEdges[edgeIndex*HALF_EDGE_FIELDS+1];
    assertIndex(beforeEdge,halfEdgeCount,'beforeEdge'); assertIndex(nextEdge,halfEdgeCount,'nextEdge');
    const p0=halfEdges[edgeIndex*HALF_EDGE_FIELDS];
    const p1=halfEdges[beforeEdge*HALF_EDGE_FIELDS];
    const p2=halfEdges[nextEdge*HALF_EDGE_FIELDS];
    assertIndex(p0,vertexCount,'endPos'); assertIndex(p1,vertexCount,'endPos'); assertIndex(p2,vertexCount,'endPos');
    indices.set([p0,p1,p2],face*3);
  }
  return {version,vertexCount,halfEdgeCount,faceCount,positions,indices,halfEdges,positionToEdge,faceToEdge};
}

function assertIndex(index:number,length:number,field:string){
  if(index<0||index>=length) throw new Error(`${field}に範囲外の参照があります: ${index}`);
}
