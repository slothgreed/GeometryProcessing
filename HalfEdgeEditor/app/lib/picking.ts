import type { HalfEdgeMeshData } from './halfEdgeLoader';

const HALF_EDGE_FIELDS = 5;
const END_POSITION_FIELD = 0;
const BEFORE_EDGE_FIELD = 2;
const OPPOSITE_EDGE_FIELD = 3;

export function createPickingIdColors(
    elementCount: number,
    verticesPerElement: number,
    resolveElementId: (elementIndex: number) => number = (elementIndex) => elementIndex,
) {
    const colors = new Uint8Array(elementCount * verticesPerElement * 3);

    for (let elementIndex = 0; elementIndex < elementCount; elementIndex += 1) {
        const encodedId = resolveElementId(elementIndex) + 1;

        for (let vertexIndex = 0; vertexIndex < verticesPerElement; vertexIndex += 1) {
            const colorOffset = (elementIndex * verticesPerElement + vertexIndex) * 3;

            colors[colorOffset] = encodedId & 0xff;
            colors[colorOffset + 1] = (encodedId >> 8) & 0xff;
            colors[colorOffset + 2] = (encodedId >> 16) & 0xff;
        }
    }

    return colors;
}

export function decodePickingId(pixel: Uint8Array) {
    const encodedId = pixel[0] | (pixel[1] << 8) | (pixel[2] << 16);

    return encodedId === 0 ? null : encodedId - 1;
}

export function getHalfEdgeVertexIndices(data: HalfEdgeMeshData, halfEdgeIndex: number) {
    if (halfEdgeIndex < 0 || halfEdgeIndex >= data.halfEdgeCount) {
        return null;
    }

    const edgeOffset = halfEdgeIndex * HALF_EDGE_FIELDS;
    const endVertexIndex = data.halfEdges[edgeOffset + END_POSITION_FIELD];
    const oppositeEdgeIndex = data.halfEdges[edgeOffset + OPPOSITE_EDGE_FIELD];
    let beginVertexIndex = -1;

    if (oppositeEdgeIndex >= 0 && oppositeEdgeIndex < data.halfEdgeCount) {
        beginVertexIndex = data.halfEdges[oppositeEdgeIndex * HALF_EDGE_FIELDS + END_POSITION_FIELD];
    } else {
        const beforeEdgeIndex = data.halfEdges[edgeOffset + BEFORE_EDGE_FIELD];

        if (beforeEdgeIndex >= 0 && beforeEdgeIndex < data.halfEdgeCount) {
            beginVertexIndex = data.halfEdges[beforeEdgeIndex * HALF_EDGE_FIELDS + END_POSITION_FIELD];
        }
    }

    if (
        beginVertexIndex < 0
        || beginVertexIndex >= data.vertexCount
        || endVertexIndex < 0
        || endVertexIndex >= data.vertexCount
    ) {
        return null;
    }

    return [beginVertexIndex, endVertexIndex] as const;
}

export function createEdgePickingData(data: HalfEdgeMeshData) {
    const positions: number[] = [];
    const halfEdgeIndices: number[] = [];

    for (let halfEdgeIndex = 0; halfEdgeIndex < data.halfEdgeCount; halfEdgeIndex += 1) {
        const edgeOffset = halfEdgeIndex * HALF_EDGE_FIELDS;
        const oppositeEdgeIndex = data.halfEdges[edgeOffset + OPPOSITE_EDGE_FIELD];

        if (oppositeEdgeIndex >= 0 && halfEdgeIndex > oppositeEdgeIndex) {
            continue;
        }

        const vertexIndices = getHalfEdgeVertexIndices(data, halfEdgeIndex);

        if (!vertexIndices) {
            continue;
        }

        for (const vertexIndex of vertexIndices) {
            const positionOffset = vertexIndex * 3;

            positions.push(
                data.positions[positionOffset],
                data.positions[positionOffset + 1],
                data.positions[positionOffset + 2],
            );
        }

        halfEdgeIndices.push(halfEdgeIndex);
    }

    return {
        positions: new Float32Array(positions),
        halfEdgeIndices,
    };
}

export function createFacePickingPositions(data: HalfEdgeMeshData) {
    const positions = new Float32Array(data.faceCount * 9);

    for (let faceIndex = 0; faceIndex < data.faceCount; faceIndex += 1) {
        for (let faceVertexIndex = 0; faceVertexIndex < 3; faceVertexIndex += 1) {
            const vertexIndex = data.indices[faceIndex * 3 + faceVertexIndex];
            const sourceOffset = vertexIndex * 3;
            const destinationOffset = (faceIndex * 3 + faceVertexIndex) * 3;

            positions[destinationOffset] = data.positions[sourceOffset];
            positions[destinationOffset + 1] = data.positions[sourceOffset + 1];
            positions[destinationOffset + 2] = data.positions[sourceOffset + 2];
        }
    }

    return positions;
}
