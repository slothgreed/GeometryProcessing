import type { HalfEdgeMeshData } from './halfEdgeTypes';
import { getHalfEdgeVertexIndices } from './picking';

export type VertexInfluence = {
    vertexIndex: number;
    weight: number;
};

type Neighbor = {
    vertexIndex: number;
    distance: number;
};

type DistanceCandidate = {
    vertexIndex: number;
    distance: number;
};

const DECAY_DISTANCE_RATE = 0.035;
const MINIMUM_INFLUENCE = 0.05;

export function createVertexInfluences(
    data: HalfEdgeMeshData,
    selectedVertexIndex: number,
    meshRadius: number,
) {
    const decayDistance = Math.max(meshRadius * DECAY_DISTANCE_RATE, Number.EPSILON);
    const maximumDistance = -Math.log(MINIMUM_INFLUENCE) * decayDistance;
    const adjacency = createVertexAdjacency(data);
    const distances = new Float64Array(data.vertexCount);
    const candidates: DistanceCandidate[] = [{
        vertexIndex: selectedVertexIndex,
        distance: 0,
    }];
    const influences: VertexInfluence[] = [];

    distances.fill(Number.POSITIVE_INFINITY);
    distances[selectedVertexIndex] = 0;

    while (candidates.length > 0) {
        candidates.sort((left, right) => right.distance - left.distance);

        const candidate = candidates.pop();

        if (!candidate || candidate.distance !== distances[candidate.vertexIndex]) {
            continue;
        }

        if (candidate.distance > maximumDistance) {
            continue;
        }

        influences.push({
            vertexIndex: candidate.vertexIndex,
            weight: Math.exp(-candidate.distance / decayDistance),
        });

        for (const neighbor of adjacency[candidate.vertexIndex]) {
            const nextDistance = candidate.distance + neighbor.distance;

            if (nextDistance >= distances[neighbor.vertexIndex] || nextDistance > maximumDistance) {
                continue;
            }

            distances[neighbor.vertexIndex] = nextDistance;
            candidates.push({
                vertexIndex: neighbor.vertexIndex,
                distance: nextDistance,
            });
        }
    }

    return influences;
}

function createVertexAdjacency(data: HalfEdgeMeshData) {
    const adjacency = Array.from({ length: data.vertexCount }, () => new Array<Neighbor>());

    for (let halfEdgeIndex = 0; halfEdgeIndex < data.halfEdgeCount; halfEdgeIndex += 1) {
        const oppositeEdgeIndex = data.halfEdges[halfEdgeIndex].oppositeEdge;

        if (oppositeEdgeIndex >= 0 && halfEdgeIndex > oppositeEdgeIndex) {
            continue;
        }

        const vertexIndices = getHalfEdgeVertexIndices(data, halfEdgeIndex);

        if (!vertexIndices) {
            continue;
        }

        const [beginVertexIndex, endVertexIndex] = vertexIndices;
        const distance = getVertexDistance(data, beginVertexIndex, endVertexIndex);

        adjacency[beginVertexIndex].push({
            vertexIndex: endVertexIndex,
            distance,
        });
        adjacency[endVertexIndex].push({
            vertexIndex: beginVertexIndex,
            distance,
        });
    }

    return adjacency;
}

function getVertexDistance(data: HalfEdgeMeshData, firstVertexIndex: number, secondVertexIndex: number) {
    const firstOffset = firstVertexIndex * 3;
    const secondOffset = secondVertexIndex * 3;
    const differenceX = data.positions[firstOffset] - data.positions[secondOffset];
    const differenceY = data.positions[firstOffset + 1] - data.positions[secondOffset + 1];
    const differenceZ = data.positions[firstOffset + 2] - data.positions[secondOffset + 2];

    return Math.hypot(differenceX, differenceY, differenceZ);
}
