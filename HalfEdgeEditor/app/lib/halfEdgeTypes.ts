export type HalfEdge = {
    endPosition: number;
    nextEdge: number;
    beforeEdge: number;
    oppositeEdge: number;
    face: number;
};

export type HalfEdgeMeshData = {
    version: number;
    vertexCount: number;
    edgeCount: number;
    halfEdgeCount: number;
    faceCount: number;
    positions: Float32Array;
    indices: Uint32Array;
    halfEdges: HalfEdge[];
    positionToEdge: Int32Array;
    faceToEdge: Int32Array;
};
