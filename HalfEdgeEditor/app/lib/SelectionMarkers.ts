import * as THREE from 'three';
import type { Selection, ViewState } from './editorTypes';
import type { HalfEdgeMeshData } from './halfEdgeLoader';
import { getHalfEdgeVertexIndices } from './picking';

type MarkerState = {
    selection: Selection | null;
    hoverSelection: Selection | null;
    view: ViewState;
};

export default class SelectionMarkers {
    private readonly scene: THREE.Scene;
    private readonly data: HalfEdgeMeshData;
    private readonly vertexGeometry: THREE.BufferGeometry;
    private readonly vertexMaterial: THREE.PointsMaterial;
    private readonly vertex: THREE.Points;
    private readonly edgeGeometry: THREE.BufferGeometry;
    private readonly edgeMaterial: THREE.LineBasicMaterial;
    private readonly edge: THREE.LineSegments;
    private readonly faceGeometry: THREE.BufferGeometry;
    private readonly faceMaterial: THREE.MeshBasicMaterial;
    private readonly face: THREE.Mesh;

    constructor(scene: THREE.Scene, data: HalfEdgeMeshData, meshRadius: number) {
        this.scene = scene;
        this.data = data;

        this.vertexGeometry = new THREE.BufferGeometry();
        this.vertexGeometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(3), 3));

        this.vertexMaterial = new THREE.PointsMaterial({
            color: 0xffc857,
            size: meshRadius * 0.034,
            sizeAttenuation: true,
            depthTest: true,
            depthWrite: false,
            toneMapped: false,
        });

        this.vertex = new THREE.Points(this.vertexGeometry, this.vertexMaterial);
        this.vertex.visible = false;
        this.vertex.renderOrder = 3;
        this.scene.add(this.vertex);

        this.edgeGeometry = new THREE.BufferGeometry();
        this.edgeGeometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(6), 3));

        this.edgeMaterial = new THREE.LineBasicMaterial({
            color: 0xffc857,
            depthTest: true,
            depthWrite: false,
            toneMapped: false,
        });

        this.edge = new THREE.LineSegments(this.edgeGeometry, this.edgeMaterial);
        this.edge.visible = false;
        this.edge.renderOrder = 3;
        this.scene.add(this.edge);

        this.faceGeometry = new THREE.BufferGeometry();
        this.faceGeometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(9), 3));

        this.faceMaterial = new THREE.MeshBasicMaterial({
            color: 0xffc857,
            transparent: true,
            opacity: 0.55,
            side: THREE.DoubleSide,
            depthTest: true,
            depthWrite: false,
            toneMapped: false,
        });

        this.face = new THREE.Mesh(this.faceGeometry, this.faceMaterial);
        this.face.visible = false;
        this.face.renderOrder = 3;
        this.scene.add(this.face);
    }

    update({ selection, hoverSelection, view }: MarkerState) {
        const displayedSelection = hoverSelection ?? selection;
        const markerColor = hoverSelection ? 0x55d6be : 0xffc857;

        this.vertex.visible = false;
        this.edge.visible = false;
        this.face.visible = false;

        if (displayedSelection?.type === 'vertex') {
            this.updateVertex(displayedSelection.index, markerColor, view.vertices);
        }

        if (displayedSelection?.type === 'edge') {
            this.updateEdge(displayedSelection.index, markerColor, view.wireframe);
        }

        if (displayedSelection?.type === 'face') {
            this.updateFace(displayedSelection.index, markerColor, view.mesh);
        }
    }

    dispose() {
        this.scene.remove(this.vertex);
        this.scene.remove(this.edge);
        this.scene.remove(this.face);
        this.vertexGeometry.dispose();
        this.vertexMaterial.dispose();
        this.edgeGeometry.dispose();
        this.edgeMaterial.dispose();
        this.faceGeometry.dispose();
        this.faceMaterial.dispose();
    }

    private updateVertex(index: number, color: number, visible: boolean) {
        const position = this.vertexGeometry.getAttribute('position') as THREE.BufferAttribute;
        const positionOffset = index * 3;

        position.setXYZ(0, this.data.positions[positionOffset], this.data.positions[positionOffset + 1], this.data.positions[positionOffset + 2]);
        position.needsUpdate = true;
        this.vertexMaterial.color.setHex(color);
        this.vertex.visible = visible;
    }

    private updateEdge(index: number, color: number, visible: boolean) {
        const vertexIndices = getHalfEdgeVertexIndices(this.data, index);

        if (!vertexIndices) {
            return;
        }

        const position = this.edgeGeometry.getAttribute('position') as THREE.BufferAttribute;

        vertexIndices.forEach((vertexIndex, markerVertexIndex) => {
            const positionOffset = vertexIndex * 3;

            position.setXYZ(
                markerVertexIndex,
                this.data.positions[positionOffset],
                this.data.positions[positionOffset + 1],
                this.data.positions[positionOffset + 2],
            );
        });

        position.needsUpdate = true;
        this.edgeMaterial.color.setHex(color);
        this.edge.visible = visible;
    }

    private updateFace(index: number, color: number, visible: boolean) {
        const position = this.faceGeometry.getAttribute('position') as THREE.BufferAttribute;

        for (let faceVertexIndex = 0; faceVertexIndex < 3; faceVertexIndex += 1) {
            const vertexIndex = this.data.indices[index * 3 + faceVertexIndex];
            const positionOffset = vertexIndex * 3;

            position.setXYZ(
                faceVertexIndex,
                this.data.positions[positionOffset],
                this.data.positions[positionOffset + 1],
                this.data.positions[positionOffset + 2],
            );
        }

        position.needsUpdate = true;
        this.faceMaterial.color.setHex(color);
        this.face.visible = visible;
    }
}
