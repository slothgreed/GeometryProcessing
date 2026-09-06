import * as THREE from 'three';
import { LineMaterial } from 'three/addons/lines/LineMaterial.js';
import { LineSegments2 } from 'three/addons/lines/LineSegments2.js';
import { LineSegmentsGeometry } from 'three/addons/lines/LineSegmentsGeometry.js';
import type { EditMode, Selection } from './editorTypes';
import type { HalfEdgeMeshData } from './halfEdgeLoader';
import {
    createEdgePickingData,
    createFacePickingPositions,
    createPickingIdColors,
    decodePickingId,
} from './picking';

type PickRequest = {
    renderer: THREE.WebGLRenderer;
    camera: THREE.Camera;
    clientX: number;
    clientY: number;
    editMode: EditMode;
    surfaceVisible: boolean;
    wireframeVisible: boolean;
    verticesVisible: boolean;
};

export default class GpuPicker {
    private readonly data: HalfEdgeMeshData;
    private readonly scene: THREE.Scene;
    private readonly surface: THREE.Mesh;
    private readonly vertices: THREE.Points;
    private readonly edges: LineSegments2;
    private readonly faces: THREE.Mesh;
    private readonly target: THREE.WebGLRenderTarget;
    private readonly pixel = new Uint8Array(4);
    private readonly depthMaterial: THREE.MeshBasicMaterial;
    private readonly verticesGeometry: THREE.BufferGeometry;
    private readonly verticesMaterial: THREE.PointsMaterial;
    private readonly edgesGeometry: LineSegmentsGeometry;
    private readonly edgesMaterial: LineMaterial;
    private readonly facesGeometry: THREE.BufferGeometry;
    private readonly facesMaterial: THREE.MeshBasicMaterial;

    constructor(data: HalfEdgeMeshData, surfaceGeometry: THREE.BufferGeometry, meshRadius: number) {
        this.data = data;
        this.scene = new THREE.Scene();
        this.scene.background = new THREE.Color(0x000000);

        this.depthMaterial = new THREE.MeshBasicMaterial({
            colorWrite: false,
            depthWrite: true,
            side: THREE.DoubleSide,
            polygonOffset: true,
            polygonOffsetFactor: 1,
            polygonOffsetUnits: 1,
        });

        this.surface = new THREE.Mesh(surfaceGeometry, this.depthMaterial);
        this.surface.renderOrder = 0;
        this.scene.add(this.surface);

        this.verticesGeometry = new THREE.BufferGeometry();
        this.verticesGeometry.setAttribute('position', new THREE.BufferAttribute(data.positions, 3));
        this.verticesGeometry.setAttribute('color', new THREE.Uint8BufferAttribute(createPickingIdColors(data.vertexCount, 1), 3, true));

        this.verticesMaterial = new THREE.PointsMaterial({
            color: 0xffffff,
            size: meshRadius * 0.018,
            sizeAttenuation: true,
            vertexColors: true,
            depthTest: true,
            depthWrite: true,
            blending: THREE.NoBlending,
            toneMapped: false,
        });

        this.vertices = new THREE.Points(this.verticesGeometry, this.verticesMaterial);
        this.vertices.renderOrder = 1;
        this.scene.add(this.vertices);

        const edgePickingData = createEdgePickingData(data);
        const edgeIdColors = createPickingIdColors(
            edgePickingData.halfEdgeIndices.length,
            2,
            (edgeIndex) => edgePickingData.halfEdgeIndices[edgeIndex],
        );
        const edgeColors = Float32Array.from(edgeIdColors, (color) => color / 255);

        this.edgesGeometry = new LineSegmentsGeometry();
        this.edgesGeometry.setPositions(edgePickingData.positions);
        this.edgesGeometry.setColors(edgeColors);

        this.edgesMaterial = new LineMaterial({
            color: 0xffffff,
            vertexColors: true,
            linewidth: 10,
            worldUnits: false,
            depthTest: true,
            depthWrite: true,
            blending: THREE.NoBlending,
            toneMapped: false,
        });

        this.edges = new LineSegments2(this.edgesGeometry, this.edgesMaterial);
        this.edges.visible = false;
        this.edges.renderOrder = 1;
        this.scene.add(this.edges);

        this.facesGeometry = new THREE.BufferGeometry();
        this.facesGeometry.setAttribute('position', new THREE.BufferAttribute(createFacePickingPositions(data), 3));
        this.facesGeometry.setAttribute('color', new THREE.Uint8BufferAttribute(createPickingIdColors(data.faceCount, 3), 3, true));

        this.facesMaterial = new THREE.MeshBasicMaterial({
            color: 0xffffff,
            vertexColors: true,
            side: THREE.DoubleSide,
            depthTest: true,
            depthWrite: true,
            blending: THREE.NoBlending,
            toneMapped: false,
        });

        this.faces = new THREE.Mesh(this.facesGeometry, this.facesMaterial);
        this.faces.visible = false;
        this.faces.renderOrder = 1;
        this.scene.add(this.faces);

        this.target = new THREE.WebGLRenderTarget(1, 1, {
            minFilter: THREE.NearestFilter,
            magFilter: THREE.NearestFilter,
            format: THREE.RGBAFormat,
            type: THREE.UnsignedByteType,
            depthBuffer: true,
            stencilBuffer: false,
        });
        this.target.texture.colorSpace = THREE.NoColorSpace;
        this.target.texture.generateMipmaps = false;
    }

    setSize(width: number, height: number) {
        this.target.setSize(width, height);
    }

    pick({
        renderer,
        camera,
        clientX,
        clientY,
        editMode,
        surfaceVisible,
        wireframeVisible,
        verticesVisible,
    }: PickRequest): Selection | null {
        if (editMode === 'object' || !this.isSelectableVisible(
            editMode,
            surfaceVisible,
            wireframeVisible,
            verticesVisible,
        )) {
            return null;
        }

        const bounds = renderer.domElement.getBoundingClientRect();
        const pixelX = Math.min(this.target.width - 1, Math.max(0, Math.floor(((clientX - bounds.left) / bounds.width) * this.target.width)));
        const pixelY = Math.min(this.target.height - 1, Math.max(0, Math.floor(((bounds.bottom - clientY) / bounds.height) * this.target.height)));
        const previousRenderTarget = renderer.getRenderTarget();

        this.surface.visible = editMode !== 'face' && surfaceVisible;
        this.vertices.visible = editMode === 'vertex';
        this.edges.visible = editMode === 'edge';
        this.faces.visible = editMode === 'face';
        renderer.setRenderTarget(this.target);

        try {
            renderer.render(this.scene, camera);
            renderer.readRenderTargetPixels(this.target, pixelX, pixelY, 1, 1, this.pixel);
        } finally {
            renderer.setRenderTarget(previousRenderTarget);
        }

        const pickedIndex = decodePickingId(this.pixel);

        if (pickedIndex === null || !this.isIndexValid(editMode, pickedIndex)) {
            return null;
        }

        return {
            type: editMode,
            index: pickedIndex,
        };
    }

    dispose() {
        this.depthMaterial.dispose();
        this.verticesGeometry.dispose();
        this.verticesMaterial.dispose();
        this.edgesGeometry.dispose();
        this.edgesMaterial.dispose();
        this.facesGeometry.dispose();
        this.facesMaterial.dispose();
        this.target.dispose();
    }

    private isSelectableVisible(
        editMode: EditMode,
        surfaceVisible: boolean,
        wireframeVisible: boolean,
        verticesVisible: boolean,
    ) {
        return (
            editMode === 'vertex' && verticesVisible
        ) || (
            editMode === 'edge' && wireframeVisible
        ) || (
            editMode === 'face' && surfaceVisible
        );
    }

    private isIndexValid(editMode: EditMode, index: number) {
        return (
            editMode === 'vertex' && index < this.data.vertexCount
        ) || (
            editMode === 'edge' && index < this.data.halfEdgeCount
        ) || (
            editMode === 'face' && index < this.data.faceCount
        );
    }
}
