import * as THREE from 'three';
import type { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import GpuPicker from './GpuPicker';
import type { EditMode, Selection } from './editorTypes';
import type { HalfEdgeMeshData } from './halfEdgeLoader';

type SelectionControllerOptions = {
    data: HalfEdgeMeshData;
    renderer: THREE.WebGLRenderer;
    camera: THREE.Camera;
    controls: OrbitControls;
    editMode: EditMode;
    surfaceGeometry: THREE.BufferGeometry;
    meshRadius: number;
    mesh: THREE.Mesh;
    wireframe: THREE.LineSegments;
    vertices: THREE.Points;
    onSelectionChange: (selection: Selection | null) => void;
    onHoverSelectionChange: (selection: Selection | null) => void;
};

export default class SelectionController {
    private readonly renderer: THREE.WebGLRenderer;
    private readonly camera: THREE.Camera;
    private readonly controls: OrbitControls;
    private readonly mesh: THREE.Mesh;
    private readonly wireframe: THREE.LineSegments;
    private readonly vertices: THREE.Points;
    private readonly onSelectionChange: (selection: Selection | null) => void;
    private readonly onHoverSelectionChange: (selection: Selection | null) => void;
    private readonly gpuPicker: GpuPicker;
    private readonly pointerDownPosition = new THREE.Vector2();
    private readonly hoverPointerPosition = new THREE.Vector2();
    private editMode: EditMode;
    private hoverPickingFrame = 0;
    private pointerInside = false;
    private pointerButtons = 0;
    private pickingEnabled = true;

    constructor({
        data,
        renderer,
        camera,
        controls,
        editMode,
        surfaceGeometry,
        meshRadius,
        mesh,
        wireframe,
        vertices,
        onSelectionChange,
        onHoverSelectionChange,
    }: SelectionControllerOptions) {
        this.renderer = renderer;
        this.camera = camera;
        this.controls = controls;
        this.editMode = editMode;
        this.mesh = mesh;
        this.wireframe = wireframe;
        this.vertices = vertices;
        this.onSelectionChange = onSelectionChange;
        this.onHoverSelectionChange = onHoverSelectionChange;
        this.gpuPicker = new GpuPicker(data, surfaceGeometry, meshRadius);

        this.renderer.domElement.addEventListener('pointerdown', this.handlePointerDown);
        this.renderer.domElement.addEventListener('pointermove', this.handlePointerMove);
        this.renderer.domElement.addEventListener('pointerup', this.handlePointerUp);
        this.renderer.domElement.addEventListener('pointerleave', this.handlePointerLeave);
        this.controls.addEventListener('change', this.handleControlsChange);
    }

    setEditMode(editMode: EditMode) {
        this.editMode = editMode;
        this.onSelectionChange(null);
        this.onHoverSelectionChange(null);
    }

    setSize(width: number, height: number) {
        this.gpuPicker.setSize(width, height);
    }

    updateGeometry() {
        this.gpuPicker.updateGeometry();
    }

    setPickingEnabled(enabled: boolean) {
        this.pickingEnabled = enabled;

        if (!enabled) {
            this.onHoverSelectionChange(null);
        }
    }

    dispose() {
        if (this.hoverPickingFrame !== 0) {
            cancelAnimationFrame(this.hoverPickingFrame);
        }

        this.renderer.domElement.removeEventListener('pointerdown', this.handlePointerDown);
        this.renderer.domElement.removeEventListener('pointermove', this.handlePointerMove);
        this.renderer.domElement.removeEventListener('pointerup', this.handlePointerUp);
        this.renderer.domElement.removeEventListener('pointerleave', this.handlePointerLeave);
        this.controls.removeEventListener('change', this.handleControlsChange);
        this.gpuPicker.dispose();
    }

    private pickAt(clientX: number, clientY: number) {
        return this.gpuPicker.pick({
            renderer: this.renderer,
            camera: this.camera,
            clientX,
            clientY,
            editMode: this.editMode,
            surfaceVisible: this.mesh.visible,
            wireframeVisible: this.wireframe.visible,
            verticesVisible: this.vertices.visible,
        });
    }

    private scheduleHoverPicking = () => {
        if (!this.pickingEnabled || !this.pointerInside || this.pointerButtons !== 0 || this.hoverPickingFrame !== 0) {
            return;
        }

        this.hoverPickingFrame = requestAnimationFrame(() => {
            this.hoverPickingFrame = 0;
            this.onHoverSelectionChange(this.pickAt(this.hoverPointerPosition.x, this.hoverPointerPosition.y));
        });
    };

    private handlePointerDown = (event: PointerEvent) => {
        this.pointerInside = true;
        this.pointerButtons = event.buttons;
        this.pointerDownPosition.set(event.clientX, event.clientY);
        this.hoverPointerPosition.set(event.clientX, event.clientY);
        this.onHoverSelectionChange(null);
    };

    private handlePointerMove = (event: PointerEvent) => {
        this.pointerInside = true;
        this.pointerButtons = event.buttons;
        this.hoverPointerPosition.set(event.clientX, event.clientY);

        if (event.buttons !== 0) {
            this.onHoverSelectionChange(null);

            return;
        }

        this.scheduleHoverPicking();
    };

    private handlePointerUp = (event: PointerEvent) => {
        this.pointerInside = true;
        this.pointerButtons = event.buttons;
        this.hoverPointerPosition.set(event.clientX, event.clientY);

        if (!this.pickingEnabled) {
            return;
        }

        const dragDistance = this.pointerDownPosition.distanceTo(new THREE.Vector2(event.clientX, event.clientY));

        if (dragDistance > 4) {
            this.scheduleHoverPicking();

            return;
        }

        this.onSelectionChange(this.pickAt(event.clientX, event.clientY));
        this.onHoverSelectionChange(null);
    };

    private handlePointerLeave = () => {
        this.pointerInside = false;
        this.pointerButtons = 0;

        if (this.hoverPickingFrame !== 0) {
            cancelAnimationFrame(this.hoverPickingFrame);
            this.hoverPickingFrame = 0;
        }

        this.onHoverSelectionChange(null);
    };

    private handleControlsChange = () => {
        this.scheduleHoverPicking();
    };
}
