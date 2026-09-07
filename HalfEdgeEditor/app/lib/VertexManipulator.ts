import * as THREE from 'three';
import type { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { TransformControls } from 'three/addons/controls/TransformControls.js';
import type { Selection } from './editorTypes';
import type { HalfEdgeMeshData } from './halfEdgeTypes';
import { createVertexInfluences, type VertexInfluence } from './proportionalEditing';

type VertexManipulatorOptions = {
    scene: THREE.Scene;
    camera: THREE.Camera;
    domElement: HTMLElement;
    orbitControls: OrbitControls;
    data: HalfEdgeMeshData;
    meshRadius: number;
    onVertexChange: (vertexIndex: number) => void;
    onDraggingChange: (dragging: boolean) => void;
};

export default class VertexManipulator {
    private readonly scene: THREE.Scene;
    private readonly orbitControls: OrbitControls;
    private readonly data: HalfEdgeMeshData;
    private readonly meshRadius: number;
    private readonly onVertexChange: (vertexIndex: number) => void;
    private readonly onDraggingChange: (dragging: boolean) => void;
    private readonly controls: TransformControls;
    private readonly target = new THREE.Object3D();
    private readonly dragStartPosition = new THREE.Vector3();
    private readonly dragOffset = new THREE.Vector3();
    private vertexIndex: number | null = null;
    private influences: VertexInfluence[] = [];
    private originalPositions = new Float32Array();

    constructor({
        scene,
        camera,
        domElement,
        orbitControls,
        data,
        meshRadius,
        onVertexChange,
        onDraggingChange,
    }: VertexManipulatorOptions) {
        this.scene = scene;
        this.orbitControls = orbitControls;
        this.data = data;
        this.meshRadius = meshRadius;
        this.onVertexChange = onVertexChange;
        this.onDraggingChange = onDraggingChange;
        this.scene.add(this.target);

        this.controls = new TransformControls(camera, domElement);
        this.controls.setMode('translate');
        this.controls.setSpace('world');
        this.controls.setSize(0.72);
        this.controls.addEventListener('mouseDown', this.handleMouseDown);
        this.controls.addEventListener('dragging-changed', this.handleDraggingChanged);
        this.controls.addEventListener('objectChange', this.handleObjectChange);
        this.scene.add(this.controls.getHelper());
    }

    setSelection(selection: Selection | null) {
        if (selection?.type !== 'vertex') {
            this.vertexIndex = null;
            this.influences = [];
            this.originalPositions = new Float32Array();
            this.controls.detach();

            return;
        }

        this.vertexIndex = selection.index;
        this.influences = createVertexInfluences(this.data, selection.index, this.meshRadius);
        const positionOffset = selection.index * 3;

        this.target.position.set(
            this.data.positions[positionOffset],
            this.data.positions[positionOffset + 1],
            this.data.positions[positionOffset + 2],
        );
        this.controls.attach(this.target);
    }

    dispose() {
        this.controls.removeEventListener('mouseDown', this.handleMouseDown);
        this.controls.removeEventListener('dragging-changed', this.handleDraggingChanged);
        this.controls.removeEventListener('objectChange', this.handleObjectChange);
        this.controls.detach();
        this.scene.remove(this.controls.getHelper());
        this.scene.remove(this.target);
        this.controls.dispose();
    }

    private handleDraggingChanged = (event: THREE.Event & { value?: unknown }) => {
        const dragging = event.value === true;

        this.orbitControls.enabled = !dragging;
        this.onDraggingChange(dragging);
    };

    private handleMouseDown = () => {
        this.dragStartPosition.copy(this.target.position);
        this.originalPositions = new Float32Array(this.influences.length * 3);

        this.influences.forEach((influence, influenceIndex) => {
            const sourceOffset = influence.vertexIndex * 3;
            const destinationOffset = influenceIndex * 3;

            this.originalPositions[destinationOffset] = this.data.positions[sourceOffset];
            this.originalPositions[destinationOffset + 1] = this.data.positions[sourceOffset + 1];
            this.originalPositions[destinationOffset + 2] = this.data.positions[sourceOffset + 2];
        });
    };

    private handleObjectChange = () => {
        if (this.vertexIndex === null) {
            return;
        }

        this.dragOffset.copy(this.target.position).sub(this.dragStartPosition);

        this.influences.forEach((influence, influenceIndex) => {
            const positionOffset = influence.vertexIndex * 3;
            const originalOffset = influenceIndex * 3;

            this.data.positions[positionOffset] = this.originalPositions[originalOffset]
                + this.dragOffset.x * influence.weight;
            this.data.positions[positionOffset + 1] = this.originalPositions[originalOffset + 1]
                + this.dragOffset.y * influence.weight;
            this.data.positions[positionOffset + 2] = this.originalPositions[originalOffset + 2]
                + this.dragOffset.z * influence.weight;
        });

        this.onVertexChange(this.vertexIndex);
    };
}
