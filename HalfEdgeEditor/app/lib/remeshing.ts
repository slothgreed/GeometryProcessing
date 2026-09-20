import type { HalfEdge, HalfEdgeMeshData } from './halfEdgeTypes';

type Triangle = [number, number, number];
type Vector = [number, number, number];

const ANGLE_TOLERANCE = 1e-8;
const FEATURE_COSINE = Math.cos(Math.PI / 6);
const SKINNY_ANGLE = 20 * Math.PI / 180;

function difference(positions: Float32Array, a: number, b: number): Vector {
    return [positions[a * 3] - positions[b * 3], positions[a * 3 + 1] - positions[b * 3 + 1], positions[a * 3 + 2] - positions[b * 3 + 2]];
}

function dot(a: Vector, b: Vector): number {
    return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

function cross(a: Vector, b: Vector): Vector {
    return [a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]];
}

function normal(positions: Float32Array, [a, b, c]: Triangle): Vector {
    return cross(difference(positions, b, a), difference(positions, c, a));
}

function aligned(a: Vector, b: Vector): boolean {
    return dot(a, b) > FEATURE_COSINE * Math.hypot(...a) * Math.hypot(...b);
}

function angle(positions: Float32Array, center: number, a: number, b: number): number {
    const u = difference(positions, a, center);
    const v = difference(positions, b, center);
    return Math.atan2(Math.hypot(...cross(u, v)), dot(u, v));
}

function minimumAngle(positions: Float32Array, triangle: Triangle): number {
    return Math.min(...triangle.map((vertex, local) => angle(positions, vertex, triangle[(local + 1) % 3], triangle[(local + 2) % 3])));
}

export function getTriangleQuality(data: HalfEdgeMeshData) {
    let minimum = Math.PI;
    let skinnyFaces = 0;

    for (let offset = 0; offset < data.indices.length; offset += 3) {
        const value = minimumAngle(data.positions, [data.indices[offset], data.indices[offset + 1], data.indices[offset + 2]]);
        minimum = Math.min(minimum, value);

        if (value < SKINNY_ANGLE) {
            skinnyFaces += 1;
        }
    }

    return {
        minimumDegrees: minimum * 180 / Math.PI,
        skinnyFaces,
    };
}

/** Collapse the shortest edge of a skinny face to an existing endpoint. */
export function collapseSkinnyTriangles(data: HalfEdgeMeshData, targetArea: number, maxPasses = 8) {
    if (!Number.isFinite(targetArea) || targetArea <= 0
        || !Number.isInteger(maxPasses) || maxPasses < 1 || maxPasses > 50) {
        throw new Error('細長い面の改善の目標面積または反復回数が不正です。');
    }

    rebuildHalfEdges(data.positions, data.indices, data.version);
    const positions = data.positions;
    let triangles: Triangle[] = [];

    for (let offset = 0; offset < data.indices.length; offset += 3) {
        triangles.push([data.indices[offset], data.indices[offset + 1], data.indices[offset + 2]]);
    }

    const protectedVertices = new Set<number>();
    let collapses = 0;
    let stoppedAtLimit = false;

    for (let pass = 0; pass < maxPasses; pass += 1) {
        const incident = Array.from({ length: data.vertexCount }, () => new Set<number>());
        const neighbors = Array.from({ length: data.vertexCount }, () => new Set<number>());
        const edges = new Map<string, number[]>();
        const faceKeys = new Map<string, number>();
        const faceAngles = triangles.map((triangle) => minimumAngle(positions, triangle));

        triangles.forEach((triangle, face) => {
            faceKeys.set([...triangle].sort((a, b) => a - b).join(':'), face);

            for (let local = 0; local < 3; local += 1) {
                const a = triangle[local];
                const b = triangle[(local + 1) % 3];
                incident[a].add(face);
                neighbors[a].add(b);
                neighbors[b].add(a);
                const key = edgeKey(a, b);
                const faces = edges.get(key) ?? [];
                faces.push(face);
                edges.set(key, faces);
            }
        });

        for (const [key, faces] of edges) {
            if (faces.length !== 2 || !aligned(normal(positions, triangles[faces[0]]), normal(positions, triangles[faces[1]]))) {
                const [a, b] = key.split(':').map(Number);
                protectedVertices.add(a);
                protectedVertices.add(b);
            }
        }

        // A vertex link must be a single cycle (interior) or path (boundary).
        const hasManifoldLink = (vertex: number): boolean => {
            const link = new Map<number, number[]>();

            for (const face of incident[vertex]) {
                const [a, b] = triangles[face].filter((value) => value !== vertex);
                link.set(a, [...(link.get(a) ?? []), b]);
                link.set(b, [...(link.get(b) ?? []), a]);
            }

            const degrees = Array.from(link.values(), (values) => values.length);
            const ends = degrees.filter((degree) => degree === 1).length;

            if (link.size === 0 || degrees.some((degree) => degree < 1 || degree > 2) || (ends !== 0 && ends !== 2)) {
                return false;
            }

            const visited = new Set<number>();
            const pending = [link.keys().next().value!];

            while (pending.length > 0) {
                const next = pending.pop()!;

                if (!visited.has(next)) {
                    visited.add(next);
                    pending.push(...link.get(next)!);
                }
            }

            return visited.size === link.size;
        };

        const locked = new Set<number>();
        const deleted = new Set<number>();
        const candidates = faceAngles.map((value, face) => ({
            value,
            face,
        })).filter((candidate) => candidate.value < SKINNY_ANGLE).sort((a, b) => a.value - b.value);
        const previousCollapses = collapses;

        for (const candidate of candidates) {
            const triangle = triangles[candidate.face];

            if (deleted.has(candidate.face) || triangle.some((vertex) => locked.has(vertex))) {
                continue;
            }

            let shortest = 0;

            for (let local = 1; local < 3; local += 1) {
                if (Math.hypot(...difference(positions, triangle[local], triangle[(local + 1) % 3]))
                    < Math.hypot(...difference(positions, triangle[shortest], triangle[(shortest + 1) % 3]))) {
                    shortest = local;
                }
            }

            const a = triangle[shortest];
            const b = triangle[(shortest + 1) % 3];
            const shared = edges.get(edgeKey(a, b))!;
            const ring = new Set([...neighbors[a], ...neighbors[b], a, b]);

            if (shared.length !== 2 || Array.from(ring).some((vertex) => locked.has(vertex))
                || !hasManifoldLink(a) || !hasManifoldLink(b)) {
                continue;
            }

            const opposite = new Set(shared.flatMap((face) => triangles[face].filter((vertex) => vertex !== a && vertex !== b)));
            const common = Array.from(neighbors[a]).filter((vertex) => neighbors[b].has(vertex));

            // Link condition: no extra common neighbors may be identified by collapse.
            if (opposite.size !== 2 || common.length !== 2 || common.some((vertex) => !opposite.has(vertex))) {
                continue;
            }

            let best: { replacements: Map<number, Triangle>; score: number } | null = null;

            for (const [remove, keep] of [[a, b], [b, a]]) {
                if (protectedVertices.has(remove)) {
                    continue;
                }

                const replacements = new Map<number, Triangle>();
                const replacementKeys = new Set<string>();
                let score = Math.PI;
                let valid = true;

                for (const face of incident[remove]) {
                    if (shared.includes(face)) {
                        continue;
                    }

                    const old = triangles[face];
                    const replacement = old.map((vertex) => vertex === remove ? keep : vertex) as Triangle;
                    const key = [...replacement].sort((left, right) => left - right).join(':');
                    const existing = faceKeys.get(key);
                    const newAngle = minimumAngle(positions, replacement);

                    // Reject duplicate faces (including the tetrahedron link case), flips,
                    // and any surviving face that becomes skinny or more skinny.
                    if ((existing !== undefined && existing !== face) || replacementKeys.has(key)
                        || !aligned(normal(positions, old), normal(positions, replacement))
                        || newAngle + ANGLE_TOLERANCE < Math.min(faceAngles[face], SKINNY_ANGLE)
                        || area(positions, replacement) > Math.max(targetArea, area(positions, old))) {
                        valid = false;
                        break;
                    }

                    score = Math.min(score, newAngle);
                    replacements.set(face, replacement);
                    replacementKeys.add(key);
                }

                if (valid && replacements.size > 0 && (!best || score > best.score)) {
                    best = {
                        replacements,
                        score,
                    };
                }
            }

            if (!best) {
                continue;
            }

            for (const face of shared) {
                deleted.add(face);
            }

            for (const [face, replacement] of best.replacements) {
                triangles[face] = replacement;
            }

            // Disjoint one-rings make the pass's adjacency valid for later candidates.
            for (const vertex of ring) {
                locked.add(vertex);
            }

            collapses += 1;
        }

        triangles = triangles.filter((_, face) => !deleted.has(face));

        if (collapses === previousCollapses) {
            break;
        }

        stoppedAtLimit = pass === maxPasses - 1;
    }

    if (collapses === 0) {
        return {
            data,
            collapses,
            stoppedAtLimit,
        };
    }

    // Compact removed vertices; no retained undo mesh or dangling representative edges.
    const remap = new Map<number, number>();
    const compactPositions: number[] = [];
    const indices = triangles.flat().map((vertex) => {
        let mapped = remap.get(vertex);

        if (mapped === undefined) {
            mapped = remap.size;
            remap.set(vertex, mapped);
            compactPositions.push(positions[vertex * 3], positions[vertex * 3 + 1], positions[vertex * 3 + 2]);
        }

        return mapped;
    });

    return {
        data: rebuildHalfEdges(new Float32Array(compactPositions), new Uint32Array(indices), data.version),
        collapses,
        stoppedAtLimit,
    };
}

/** Conservative extrinsic flips, not an intrinsic triangulation of the original surface. */
export function improveDelaunay(data: HalfEdgeMeshData, targetArea = Infinity, maxPasses = 10) {
    if (!(targetArea > 0) || !Number.isInteger(maxPasses) || maxPasses < 1 || maxPasses > 50) {
        throw new Error('辺の改善の目標面積または反復回数が不正です。');
    }

    rebuildHalfEdges(data.positions, data.indices, data.version);
    const positions = data.positions;
    const triangles: Triangle[] = [];
    const directed = new Map<string, number>();
    const protectedEdges = new Set<string>();

    const register = (face: number, triangle: Triangle) => {
        triangles[face] = triangle;

        for (let local = 0; local < 3; local += 1) {
            directed.set(`${triangle[local]}:${triangle[(local + 1) % 3]}`, face);
        }
    };

    for (let offset = 0; offset < data.indices.length; offset += 3) {
        register(offset / 3, [data.indices[offset], data.indices[offset + 1], data.indices[offset + 2]]);
    }

    // Keep original creases fixed even if adjacent face normals change later.
    for (const triangle of triangles) {
        for (let local = 0; local < 3; local += 1) {
            const a = triangle[local];
            const b = triangle[(local + 1) % 3];
            const neighbor = directed.get(`${b}:${a}`);

            if (neighbor === undefined || !aligned(normal(positions, triangle), normal(positions, triangles[neighbor]))) {
                protectedEdges.add(edgeKey(a, b));
            }
        }
    }

    let flips = 0;
    let stoppedAtLimit = false;

    for (let pass = 0; pass < maxPasses; pass += 1) {
        const previousFlips = flips;

        for (let face = 0; face < triangles.length; face += 1) {
            for (let local = 0; local < 3; local += 1) {
                const first = triangles[face];
                const a = first[local];
                const b = first[(local + 1) % 3];
                const c = first[(local + 2) % 3];
                const neighbor = directed.get(`${b}:${a}`);

                if (neighbor === undefined || face >= neighbor || protectedEdges.has(edgeKey(a, b))) {
                    continue;
                }

                const second = triangles[neighbor];
                const d = second.find((vertex) => vertex !== a && vertex !== b)!;

                if (c === d || directed.has(`${c}:${d}`) || directed.has(`${d}:${c}`)
                    || angle(positions, c, a, b) + angle(positions, d, a, b) <= Math.PI + ANGLE_TOLERANCE) {
                    continue;
                }

                const firstNormal = normal(positions, first);
                const secondNormal = normal(positions, second);
                const replacement: [Triangle, Triangle] = [[c, d, b], [d, c, a]];
                const normals = replacement.map((triangle) => normal(positions, triangle));
                const maximumArea = Math.max(targetArea, area(positions, first), area(positions, second));
                const oldMinimum = Math.min(minimumAngle(positions, first), minimumAngle(positions, second));
                const newMinimum = Math.min(...replacement.map((triangle) => minimumAngle(positions, triangle)));

                // Orientation checks also reject non-convex or collapsed projected quads.
                if (!aligned(firstNormal, secondNormal)
                    || normals.some((value) => !aligned(value, firstNormal) || !aligned(value, secondNormal))
                    || newMinimum <= oldMinimum + ANGLE_TOLERANCE
                    || replacement.some((triangle) => area(positions, triangle) > maximumArea)) {
                    continue;
                }

                for (const triangle of [first, second]) {
                    for (let edge = 0; edge < 3; edge += 1) {
                        directed.delete(`${triangle[edge]}:${triangle[(edge + 1) % 3]}`);
                    }
                }

                register(face, replacement[0]);
                register(neighbor, replacement[1]);
                flips += 1;
            }
        }

        if (flips === previousFlips) {
            break;
        }

        stoppedAtLimit = pass === maxPasses - 1;
    }

    return {
        data: rebuildHalfEdges(positions, new Uint32Array(triangles.flat()), data.version),
        flips,
        stoppedAtLimit,
    };
}

function edgeKey(a: number, b: number): string {
    return a < b ? `${a}:${b}` : `${b}:${a}`;
}

function area(positions: ArrayLike<number>, [a, b, c]: Triangle): number {
    const ux = positions[b * 3] - positions[a * 3];
    const uy = positions[b * 3 + 1] - positions[a * 3 + 1];
    const uz = positions[b * 3 + 2] - positions[a * 3 + 2];
    const vx = positions[c * 3] - positions[a * 3];
    const vy = positions[c * 3 + 1] - positions[a * 3 + 1];
    const vz = positions[c * 3 + 2] - positions[a * 3 + 2];
    return Math.hypot(uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx) / 2;
}

export function getAreaStatistics(data: HalfEdgeMeshData) {
    let total = 0;
    let maximum = 0;

    for (let i = 0; i < data.indices.length; i += 3) {
        const value = area(data.positions, [data.indices[i], data.indices[i + 1], data.indices[i + 2]]);
        total += value;
        maximum = Math.max(maximum, value);
    }

    return {
        mean: total / data.faceCount,
        maximum,
    };
}

// Rebuild using the loader's convention: indices = [end, before.end, next.end].
export function rebuildHalfEdges(positions: Float32Array, indices: Uint32Array, version = 2): HalfEdgeMeshData {
    if (positions.length % 3 !== 0 || indices.length === 0 || indices.length % 3 !== 0
        || !positions.every(Number.isFinite)) {
        throw new Error('頂点または三角形のデータが不正です。');
    }

    const vertexCount = positions.length / 3;
    const faceCount = indices.length / 3;
    const halfEdges: HalfEdge[] = [];
    const faceToEdge = new Int32Array(faceCount);
    const positionToEdge = new Int32Array(vertexCount).fill(-1);
    const directed = new Map<string, number>();
    let edgeCount = 0;

    for (let face = 0; face < faceCount; face += 1) {
        const base = face * 3;
        const triangle: Triangle = [indices[base], indices[base + 1], indices[base + 2]];

        if (triangle.some((vertex) => vertex >= vertexCount) || new Set(triangle).size !== 3
            || !(area(positions, triangle) > 0)) {
            throw new Error(`面 ${face} に不正な頂点参照または縮退があります。`);
        }

        const ends = [triangle[0], triangle[2], triangle[1]];
        faceToEdge[face] = base;

        for (let local = 0; local < 3; local += 1) {
            const start = ends[(local + 2) % 3];
            const end = ends[local];
            const key = `${start}:${end}`;

            if (directed.has(key)) {
                throw new Error('辺の向きが不整合、または3面以上が共有する辺があります。');
            }

            const opposite = directed.get(`${end}:${start}`) ?? -1;
            halfEdges.push({
                endPosition: end,
                nextEdge: base + (local + 1) % 3,
                beforeEdge: base + (local + 2) % 3,
                oppositeEdge: opposite,
                face,
            });
            directed.set(key, base + local);
            positionToEdge[start] = base + local;

            if (opposite >= 0) {
                halfEdges[opposite].oppositeEdge = base + local;
            } else {
                edgeCount += 1;
            }
        }
    }

    return {
        version,
        vertexCount,
        edgeCount,
        halfEdgeCount: halfEdges.length,
        faceCount,
        positions,
        indices,
        halfEdges,
        positionToEdge,
        faceToEdge,
    };
}

export function remeshByArea(data: HalfEdgeMeshData, targetArea: number, maxSplits: number) {
    if (!Number.isFinite(targetArea) || targetArea <= 0
        || !Number.isInteger(maxSplits) || maxSplits < 1 || maxSplits > 10000) {
        throw new Error('目標面積は正の数、最大分割回数は1〜10000の整数にしてください。');
    }

    // Validate before editing; all subsequent changes are made on private copies.
    rebuildHalfEdges(data.positions, data.indices, data.version);
    const positions = Array.from(data.positions);
    const triangles: Triangle[] = [];
    const areas: number[] = [];
    const adjacent = new Map<string, Set<number>>();

    const register = (face: number, triangle: Triangle) => {
        triangles[face] = triangle;
        areas[face] = area(positions, triangle);

        for (let local = 0; local < 3; local += 1) {
            const key = edgeKey(triangle[local], triangle[(local + 1) % 3]);
            const faces = adjacent.get(key) ?? new Set<number>();
            faces.add(face);
            adjacent.set(key, faces);
        }
    };

    for (let i = 0; i < data.indices.length; i += 3) {
        register(i / 3, [data.indices[i], data.indices[i + 1], data.indices[i + 2]]);
    }

    let splits = 0;

    while (splits < maxSplits) {
        let largest = -1;
        let largestArea = targetArea;

        for (let face = 0; face < areas.length; face += 1) {
            if (areas[face] > largestArea) {
                largest = face;
                largestArea = areas[face];
            }
        }

        if (largest < 0) {
            break;
        }

        const triangle = triangles[largest];
        let longest = 0;
        let longestLength = -1;

        for (let local = 0; local < 3; local += 1) {
            const a = triangle[local];
            const b = triangle[(local + 1) % 3];
            const length = Math.hypot(...[0, 1, 2].map((axis) => positions[a * 3 + axis] - positions[b * 3 + axis]));

            if (length > longestLength) {
                longest = local;
                longestLength = length;
            }
        }

        const a = triangle[longest];
        const b = triangle[(longest + 1) % 3];
        const midpoint = positions.length / 3;
        const coordinates = [0, 1, 2].map((axis) => Math.fround((positions[a * 3 + axis] + positions[b * 3 + axis]) / 2));

        if ([a, b].some((vertex) => coordinates.every((value, axis) => value === positions[vertex * 3 + axis]))) {
            throw new Error('座標の精度限界に達しました。目標面積を大きくしてください。');
        }

        positions.push(...coordinates);
        const faces = Array.from(adjacent.get(edgeKey(a, b))!);

        for (const face of faces) {
            const old = triangles[face];

            for (let local = 0; local < 3; local += 1) {
                const key = edgeKey(old[local], old[(local + 1) % 3]);
                const neighbors = adjacent.get(key)!;
                neighbors.delete(face);

                if (neighbors.size === 0) {
                    adjacent.delete(key);
                }
            }

            const local = old.findIndex((vertex, index) => edgeKey(vertex, old[(index + 1) % 3]) === edgeKey(a, b));
            const start = old[local];
            const end = old[(local + 1) % 3];
            const third = old[(local + 2) % 3];
            register(face, [start, midpoint, third]);
            register(triangles.length, [midpoint, end, third]);
        }

        splits += 1;
    }

    const result = rebuildHalfEdges(new Float32Array(positions), new Uint32Array(triangles.flat()), data.version);
    const remaining = areas.filter((value) => value > targetArea).length;
    return {
        data: result,
        splits,
        remaining,
    };
}
