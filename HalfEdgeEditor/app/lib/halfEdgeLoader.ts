import type { HalfEdge, HalfEdgeMeshData } from './halfEdgeTypes';

export type { HalfEdge, HalfEdgeMeshData } from './halfEdgeTypes';

const HEADER_BYTES = 16;
const INDEXED_VERTEX_BYTES = 16;
const HALF_EDGE_FIELDS = 5;
const HALF_EDGE_BYTES = HALF_EDGE_FIELDS * Int32Array.BYTES_PER_ELEMENT;

export function parseHalfEdgeFile(buffer: ArrayBuffer): HalfEdgeMeshData {
    if (buffer.byteLength < HEADER_BYTES) {
        throw new Error('ファイルが短すぎます。');
    }

    const view = new DataView(buffer);
    const version = view.getInt32(0, true);
    const vertexCount = view.getInt32(4, true);
    const halfEdgeCount = view.getInt32(8, true);
    const faceCount = view.getInt32(12, true);

    if (version !== 2) {
        throw new Error(`未対応のHalf-edge形式です (version ${version})。`);
    }

    if (vertexCount <= 0 || halfEdgeCount <= 0 || faceCount <= 0) {
        throw new Error('頂点・Half-edge・面の要素数が不正です。');
    }

    const expectedBytes = HEADER_BYTES
        + vertexCount * INDEXED_VERTEX_BYTES
        + halfEdgeCount * HALF_EDGE_BYTES
        + faceCount * Int32Array.BYTES_PER_ELEMENT;

    if (buffer.byteLength !== expectedBytes) {
        throw new Error(`ファイルサイズが形式と一致しません (${buffer.byteLength} / ${expectedBytes} bytes)。`);
    }

    let offset = HEADER_BYTES;
    const positions = new Float32Array(vertexCount * 3);
    const positionToEdge = new Int32Array(vertexCount);

    for (let vertexIndex = 0; vertexIndex < vertexCount; vertexIndex += 1) {
        const x = view.getFloat32(offset, true);
        const y = view.getFloat32(offset + 4, true);
        const z = view.getFloat32(offset + 8, true);

        // Match HalfEdgeLoader.cpp: Vector3(z, y, x).
        positions[vertexIndex * 3] = z;
        positions[vertexIndex * 3 + 1] = y;
        positions[vertexIndex * 3 + 2] = x;
        positionToEdge[vertexIndex] = view.getInt32(offset + 12, true);
        offset += INDEXED_VERTEX_BYTES;
    }

    const halfEdges = new Array<HalfEdge>(halfEdgeCount);
    let edgeCount = 0;

    for (let edgeIndex = 0; edgeIndex < halfEdgeCount; edgeIndex += 1) {
        const halfEdge: HalfEdge = {
            endPosition: view.getInt32(offset, true),
            nextEdge: view.getInt32(offset + 4, true),
            beforeEdge: view.getInt32(offset + 8, true),
            oppositeEdge: view.getInt32(offset + 12, true),
            face: view.getInt32(offset + 16, true),
        };
        halfEdges[edgeIndex] = halfEdge;
        offset += HALF_EDGE_BYTES;

        if (halfEdge.oppositeEdge < 0 || edgeIndex < halfEdge.oppositeEdge) {
            edgeCount += 1;
        }
    }

    const faceToEdge = new Int32Array(faceCount);
    const indices = new Uint32Array(faceCount * 3);

    for (let faceIndex = 0; faceIndex < faceCount; faceIndex += 1) {
        const edgeIndex = view.getInt32(offset, true);
        offset += Int32Array.BYTES_PER_ELEMENT;
        faceToEdge[faceIndex] = edgeIndex;
        assertIndex(edgeIndex, halfEdgeCount, 'faceToEdge');

        const edge = halfEdges[edgeIndex];
        assertIndex(edge.beforeEdge, halfEdgeCount, 'beforeEdge');
        assertIndex(edge.nextEdge, halfEdgeCount, 'nextEdge');

        const firstPosition = edge.endPosition;
        const secondPosition = halfEdges[edge.beforeEdge].endPosition;
        const thirdPosition = halfEdges[edge.nextEdge].endPosition;

        assertIndex(firstPosition, vertexCount, 'endPosition');
        assertIndex(secondPosition, vertexCount, 'endPosition');
        assertIndex(thirdPosition, vertexCount, 'endPosition');
        indices.set([firstPosition, secondPosition, thirdPosition], faceIndex * 3);
    }

    return {
        version,
        vertexCount,
        edgeCount,
        halfEdgeCount,
        faceCount,
        positions,
        indices,
        halfEdges,
        positionToEdge,
        faceToEdge,
    };
}

function assertIndex(index: number, length: number, field: string) {
    if (index < 0 || index >= length) {
        throw new Error(`${field}に範囲外の参照があります: ${index}`);
    }
}
