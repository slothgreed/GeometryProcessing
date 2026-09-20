import assert from 'node:assert/strict';
import { readFileSync } from 'node:fs';
import test from 'node:test';
import ts from 'typescript';

async function loadTypeScript(path) {
    const source = readFileSync(new URL(path, import.meta.url), 'utf8');
    const { outputText } = ts.transpileModule(source, {
        compilerOptions: {
            module: ts.ModuleKind.ESNext,
            target: ts.ScriptTarget.ES2022,
        },
    });
    return import(`data:text/javascript;base64,${Buffer.from(outputText).toString('base64')}`);
}

const { rebuildHalfEdges, remeshByArea, getAreaStatistics, improveDelaunay, collapseSkinnyTriangles, getTriangleQuality } = await loadTypeScript('../app/lib/remeshing.ts');
const { parseHalfEdgeFile } = await loadTypeScript('../app/lib/halfEdgeLoader.ts');

function verify(data) {
    assert.equal(data.halfEdgeCount, data.faceCount * 3);
    assert.equal(data.positions.length, data.vertexCount * 3);
    assert.ok(data.positions.every(Number.isFinite));

    for (let index = 0; index < data.halfEdges.length; index += 1) {
        const edge = data.halfEdges[index];
        const before = data.halfEdges[edge.beforeEdge];
        const next = data.halfEdges[edge.nextEdge];
        assert.equal(before.nextEdge, index);
        assert.equal(next.beforeEdge, index);
        assert.equal(next.face, edge.face);
        assert.equal(data.halfEdges[next.nextEdge].nextEdge, index);

        if (edge.oppositeEdge >= 0) {
            const opposite = data.halfEdges[edge.oppositeEdge];
            assert.equal(opposite.oppositeEdge, index);
            assert.equal(opposite.endPosition, before.endPosition);
            assert.equal(data.halfEdges[opposite.beforeEdge].endPosition, edge.endPosition);
        }
    }

    for (let face = 0; face < data.faceCount; face += 1) {
        const edge = data.halfEdges[data.faceToEdge[face]];
        assert.equal(edge.face, face);
        assert.deepEqual(Array.from(data.indices.slice(face * 3, face * 3 + 3)), [
            edge.endPosition,
            data.halfEdges[edge.beforeEdge].endPosition,
            data.halfEdges[edge.nextEdge].endPosition,
        ]);
    }

    data.positionToEdge.forEach((index, vertex) => {
        if (index >= 0) {
            assert.equal(data.halfEdges[data.halfEdges[index].beforeEdge].endPosition, vertex);
        }
    });
}

function mesh(positions, indices) {
    return rebuildHalfEdges(new Float32Array(positions), new Uint32Array(indices));
}

test('boundary split preserves area, winding and source', () => {
    const original = mesh([0, 0, 0, 2, 0, 0, 0, 1, 0], [0, 1, 2]);
    const snapshot = structuredClone(original);
    const result = remeshByArea(original, 0.5, 10);
    verify(result.data);
    assert.equal(result.data.faceCount, 2);
    assert.equal(result.remaining, 0);
    assert.equal(getAreaStatistics(result.data).mean, 0.5);
    assert.deepEqual(original, snapshot);

    for (let i = 0; i < result.data.indices.length; i += 3) {
        const [a, b, c] = result.data.indices.slice(i, i + 3);
        const p = result.data.positions;
        assert.ok((p[b * 3] - p[a * 3]) * (p[c * 3 + 1] - p[a * 3 + 1])
            - (p[b * 3 + 1] - p[a * 3 + 1]) * (p[c * 3] - p[a * 3]) > 0);
    }
});

test('shared diagonal gets one midpoint and no boundary crack', () => {
    const original = mesh([0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0], [0, 1, 2, 0, 2, 3]);
    const result = remeshByArea(original, 0.25, 1);
    verify(result.data);
    assert.equal(result.data.vertexCount, 5);
    assert.equal(result.data.faceCount, 4);
    assert.equal(result.data.halfEdges.filter((edge) => edge.oppositeEdge < 0).length, 4);
    assert.equal(result.data.vertexCount - result.data.edgeCount + result.data.faceCount, 1);
    assert.equal(result.remaining, 0);
});

test('split limit and invalid inputs', () => {
    const original = mesh([0, 0, 0, 2, 0, 0, 0, 1, 0], [0, 1, 2]);
    assert.equal(remeshByArea(original, 0.01, 1).remaining, 2);
    assert.equal(remeshByArea(original, 2, 1).splits, 0);
    assert.throws(() => remeshByArea(original, 0, 1));
    assert.throws(() => remeshByArea(original, 1, 1.5));
    assert.throws(() => mesh([0, 0, 0, 1, 0, 0, 2, 0, 0], [0, 1, 2]));
    assert.throws(() => rebuildHalfEdges(original.positions, new Uint32Array([0, 1, 2, 0, 1, 2])));
});

test('closed tetrahedron stays closed after refinement', () => {
    const original = mesh([0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1], [0, 2, 1, 0, 1, 3, 1, 2, 3, 2, 0, 3]);
    const result = remeshByArea(original, 0.15, 100);
    verify(result.data);
    assert.equal(result.remaining, 0);
    assert.ok(result.data.halfEdges.every((edge) => edge.oppositeEdge >= 0));
    assert.equal(result.data.vertexCount - result.data.edgeCount + result.data.faceCount, 2);
});

test('Bunny repeated refinement preserves topology and total area', () => {
    const bytes = readFileSync(new URL('../public/models/bunny6000.half', import.meta.url));
    const original = parseHalfEdgeFile(bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength));
    const stats = getAreaStatistics(original);
    let current = original;

    for (let pass = 0; pass < 2; pass += 1) {
        const result = remeshByArea(current, stats.mean, 2000);
        verify(result.data);
        assert.equal(result.data.vertexCount - result.data.edgeCount + result.data.faceCount,
            original.vertexCount - original.edgeCount + original.faceCount);
        assert.ok(getAreaStatistics(result.data).maximum <= getAreaStatistics(current).maximum * (1 + 1e-6));
        current = result.data;
    }

    assert.ok(Math.abs(getAreaStatistics(current).mean * current.faceCount - stats.mean * original.faceCount)
        < stats.mean * original.faceCount * 1e-6);
});

function edgeSet(data) {
    return new Set(data.halfEdges.map((edge) => {
        const a = edge.endPosition;
        const b = data.halfEdges[edge.beforeEdge].endPosition;
        return a < b ? `${a}:${b}` : `${b}:${a}`;
    }));
}

test('non-Delaunay diagonal flips, preserving winding, area and source', () => {
    for (const scale of [1e-6, 1, 1e6]) {
        const original = mesh([0, 0, 0, 2, 0, 0, 2, 1, 0, 0, 2, 0].map((value) => value * scale), [0, 1, 3, 1, 2, 3]);
        const snapshot = structuredClone(original);
        const result = improveDelaunay(original);
        verify(result.data);
        assert.equal(result.flips, 1);
        assert.equal(result.stoppedAtLimit, false);
        assert.ok(edgeSet(result.data).has('0:2'));
        assert.ok(!edgeSet(result.data).has('1:3'));
        assert.equal(result.data.faceCount, original.faceCount);
        assert.equal(result.data.vertexCount, original.vertexCount);
        assert.deepEqual(result.data.positions, original.positions);
        assert.deepEqual(original, snapshot);
        assert.equal(getAreaStatistics(result.data).mean, getAreaStatistics(original).mean);
        assert.equal(improveDelaunay(result.data).flips, 0);
        assert.equal(improveDelaunay(original, Infinity, 1).stoppedAtLimit, true);
    }
});

test('cocircular square and boundary triangle do not flip', () => {
    const square = mesh([0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0], [0, 1, 2, 0, 2, 3]);
    const boundary = mesh([0, 0, 0, 1, 0, 0, 0, 1, 0], [0, 1, 2]);
    assert.equal(improveDelaunay(square).flips, 0);
    assert.equal(improveDelaunay(boundary).flips, 0);
    assert.throws(() => improveDelaunay(square, NaN));
    assert.throws(() => improveDelaunay(square, 1, 0));
});

test('sharp folded non-Delaunay pair is protected', () => {
    // Fold the preceding quad by 90 degrees about its shared diagonal.
    const original = mesh([0, 0, 0, 2, 0, 0, 1.5, 0.5, Math.SQRT1_2, 0, 2, 0], [0, 1, 3, 1, 2, 3]);
    const result = improveDelaunay(original);
    assert.equal(result.flips, 0);
    assert.deepEqual(result.data.indices, original.indices);
});

test('concave quad and already-connected opposite vertices are not flipped', () => {
    const concave = mesh([0, 0, 0, 2, 0, 0, 0.5, 0.5, 0, 0, 2, 0], [0, 1, 2, 0, 2, 3]);
    const tetrahedron = mesh([0, 0, 0, 2, 0, 0, 2, 1, 0, 0, 2, 0], [0, 1, 3, 1, 2, 3, 0, 2, 1, 0, 3, 2]);
    assert.equal(improveDelaunay(concave).flips, 0);
    assert.equal(improveDelaunay(tetrahedron).flips, 0);
});

test('Bunny split then flip keeps topology, positions, boundaries and area ceiling', () => {
    const bytes = readFileSync(new URL('../public/models/bunny6000.half', import.meta.url));
    const original = parseHalfEdgeFile(bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength));
    const target = getAreaStatistics(original).mean;
    const split = remeshByArea(original, target, 2000).data;
    const result = improveDelaunay(split, target);
    verify(result.data);
    assert.ok(result.flips > 0);
    assert.equal(result.data.vertexCount, split.vertexCount);
    assert.equal(result.data.edgeCount, split.edgeCount);
    assert.equal(result.data.faceCount, split.faceCount);
    assert.deepEqual(result.data.positions, split.positions);
    assert.ok(getAreaStatistics(result.data).maximum <= Math.max(target, getAreaStatistics(split).maximum));

    for (const edge of split.halfEdges) {
        if (edge.oppositeEdge < 0) {
            const a = edge.endPosition;
            const b = split.halfEdges[edge.beforeEdge].endPosition;
            assert.ok(edgeSet(result.data).has(a < b ? `${a}:${b}` : `${b}:${a}`));
        }
    }
});

function skinnyGrid() {
    const positions = [];
    const indices = [];

    for (let y = 0; y < 5; y += 1) {
        for (let x = 0; x < 5; x += 1) {
            positions.push(x === 2 && y === 2 ? 1.05 : x, y, 0);
        }
    }

    for (let y = 0; y < 4; y += 1) {
        for (let x = 0; x < 4; x += 1) {
            const a = y * 5 + x;
            indices.push(a, a + 1, a + 6, a, a + 6, a + 5);
        }
    }

    return mesh(positions, indices);
}

function boundarySegments(data) {
    return data.halfEdges.filter((edge) => edge.oppositeEdge < 0).map((edge) => {
        const a = edge.endPosition;
        const b = data.halfEdges[edge.beforeEdge].endPosition;
        return [Array.from(data.positions.slice(a * 3, a * 3 + 3)).join(','),
            Array.from(data.positions.slice(b * 3, b * 3 + 3)).join(',')].sort().join(':');
    }).sort();
}

test('short-edge collapse removes skinny grid faces without changing boundary or area', () => {
    const original = skinnyGrid();
    const snapshot = structuredClone(original);
    const result = collapseSkinnyTriangles(original, 1);
    verify(result.data);
    assert.ok(result.collapses > 0);
    assert.equal(result.data.vertexCount, original.vertexCount - result.collapses);
    assert.equal(result.data.edgeCount, original.edgeCount - 3 * result.collapses);
    assert.equal(result.data.faceCount, original.faceCount - 2 * result.collapses);
    assert.equal(getTriangleQuality(result.data).skinnyFaces, 0);
    assert.deepEqual(boundarySegments(result.data), boundarySegments(original));
    assert.deepEqual(original, snapshot);
    assert.equal(getAreaStatistics(result.data).mean * result.data.faceCount, 16);
    assert.ok(result.data.positionToEdge.every((edge) => edge >= 0));
});

test('collapse protects boundary vertices, rejects invalid options, and is stable on a good mesh', () => {
    const boundary = mesh([0, 0, 0, 1, 0, 0, 0, 0.01, 0], [0, 1, 2]);
    assert.equal(collapseSkinnyTriangles(boundary, 1).collapses, 0);
    assert.throws(() => collapseSkinnyTriangles(boundary, 0));
    assert.throws(() => collapseSkinnyTriangles(boundary, Infinity));
    assert.throws(() => collapseSkinnyTriangles(boundary, 1, 0));
    const good = collapseSkinnyTriangles(skinnyGrid(), 1).data;
    assert.equal(collapseSkinnyTriangles(good, 1).collapses, 0);
});

test('Bunny skinny cleanup reduces bad faces and preserves topology and boundary', (context) => {
    const bytes = readFileSync(new URL('../public/models/bunny6000.half', import.meta.url));
    const original = parseHalfEdgeFile(bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.byteLength));
    const target = getAreaStatistics(original).mean;
    const refined = improveDelaunay(remeshByArea(original, target, 2000).data, target).data;
    const before = getTriangleQuality(refined);
    const result = collapseSkinnyTriangles(refined, target);
    const after = getTriangleQuality(result.data);
    verify(result.data);
    assert.ok(result.collapses > 0);
    assert.ok(after.skinnyFaces < before.skinnyFaces);
    assert.ok(after.minimumDegrees + 1e-6 >= before.minimumDegrees);
    assert.equal(result.data.vertexCount, refined.vertexCount - result.collapses);
    assert.equal(result.data.edgeCount, refined.edgeCount - 3 * result.collapses);
    assert.equal(result.data.faceCount, refined.faceCount - 2 * result.collapses);
    assert.deepEqual(boundarySegments(result.data), boundarySegments(refined));
    assert.ok(getAreaStatistics(result.data).maximum <= Math.max(target, getAreaStatistics(refined).maximum));
    context.diagnostic(`Bunny: ${result.collapses} collapses, skinny faces ${before.skinnyFaces} -> ${after.skinnyFaces}, minimum angle ${before.minimumDegrees} -> ${after.minimumDegrees}`);
});
