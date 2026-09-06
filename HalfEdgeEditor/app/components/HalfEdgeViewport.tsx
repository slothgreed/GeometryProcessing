'use client';

import { useEffect, useRef, useState } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { EDIT_MODE_LABELS, ELEMENT_LABELS, getSelectionLabel } from '../lib/editorLabels';
import type { EditMode, Selection, ViewState } from '../lib/editorTypes';
import { parseHalfEdgeFile, type HalfEdgeMeshData } from '../lib/halfEdgeLoader';
import SelectionController from '../lib/SelectionController';
import SelectionMarkers from '../lib/SelectionMarkers';

export default function HalfEdgeViewport() {
    const viewportRef = useRef<HTMLDivElement>(null);
    const fitRef = useRef<(() => void) | null>(null);
    const editModeRef = useRef<EditMode>('object');
    const meshRef = useRef<THREE.Mesh | null>(null);
    const wireframeRef = useRef<THREE.LineSegments | null>(null);
    const verticesRef = useRef<THREE.Points | null>(null);
    const selectionControllerRef = useRef<SelectionController | null>(null);
    const selectionMarkersRef = useRef<SelectionMarkers | null>(null);
    const gridRef = useRef<THREE.GridHelper | null>(null);

    const [data, setData] = useState<HalfEdgeMeshData | null>(null);
    const [error, setError] = useState<string | null>(null);
    const [editMode, setEditMode] = useState<EditMode>('object');
    const [selection, setSelection] = useState<Selection | null>(null);
    const [hoverSelection, setHoverSelection] = useState<Selection | null>(null);
    const [view, setView] = useState<ViewState>({
        mesh: true,
        wireframe: false,
        vertices: false,
        grid: true,
    });

    useEffect(() => {
        let cancelled = false;

        fetch('/models/bunny6000.half')
            .then((response) => {
                if (!response.ok) {
                    throw new Error(`Bunnyの取得に失敗しました (${response.status})。`);
                }

                return response.arrayBuffer();
            })
            .then(parseHalfEdgeFile)
            .then((loaded) => {
                if (!cancelled) {
                    setData(loaded);
                }
            })
            .catch((reason: unknown) => {
                if (!cancelled) {
                    setError(reason instanceof Error ? reason.message : String(reason));
                }
            });

        return () => {
            cancelled = true;
        };
    }, []);

    useEffect(() => {
        const host = viewportRef.current;

        if (!host || !data) {
            return;
        }

        const scene = new THREE.Scene();
        const camera = new THREE.PerspectiveCamera(38, 1, 0.001, 10000);
        const renderer = new THREE.WebGLRenderer({
            antialias: true,
            alpha: true,
        });
        renderer.setPixelRatio(Math.min(window.devicePixelRatio, 2));
        renderer.outputColorSpace = THREE.SRGBColorSpace;
        renderer.toneMapping = THREE.ACESFilmicToneMapping;
        renderer.toneMappingExposure = 1.05;
        host.appendChild(renderer.domElement);

        const geometry = new THREE.BufferGeometry();
        geometry.setAttribute('position', new THREE.BufferAttribute(data.positions, 3));
        geometry.setIndex(new THREE.BufferAttribute(data.indices, 1));
        geometry.computeVertexNormals();
        geometry.computeBoundingBox();
        geometry.computeBoundingSphere();

        const material = new THREE.MeshStandardMaterial({
            color: 0x8fb3ff,
            roughness: 0.7,
            metalness: 0.02,
            side: THREE.DoubleSide,
            polygonOffset: true,
            polygonOffsetFactor: 1,
            polygonOffsetUnits: 1,
        });

        const mesh = new THREE.Mesh(geometry, material);
        meshRef.current = mesh;
        scene.add(mesh);

        const wireframeGeometry = new THREE.WireframeGeometry(geometry);
        const wireframeMaterial = new THREE.LineBasicMaterial({
            color: 0x000000,
            depthTest: true,
            depthWrite: false,
            toneMapped: false,
        });

        const wireframe = new THREE.LineSegments(wireframeGeometry, wireframeMaterial);
        wireframe.visible = false;
        wireframe.renderOrder = 1;
        wireframeRef.current = wireframe;
        scene.add(wireframe);

        const verticesGeometry = new THREE.BufferGeometry();
        verticesGeometry.setAttribute('position', new THREE.BufferAttribute(data.positions, 3));

        const meshRadius = Math.max(geometry.boundingSphere?.radius ?? 1, 0.001);
        const verticesMaterial = new THREE.PointsMaterial({
            color: 0xe05d3d,
            size: meshRadius * 0.018,
            sizeAttenuation: true,
            depthTest: true,
            depthWrite: false,
        });

        const vertices = new THREE.Points(verticesGeometry, verticesMaterial);
        vertices.visible = false;
        vertices.renderOrder = 2;
        verticesRef.current = vertices;
        scene.add(vertices);

        const selectionMarkers = new SelectionMarkers(scene, data, meshRadius);
        selectionMarkersRef.current = selectionMarkers;

        const drawingBufferSize = new THREE.Vector2();

        const grid = new THREE.GridHelper(10, 20, 0xaab4af, 0xd5dbd8);
        grid.material.transparent = true;
        grid.material.opacity = 0.52;
        gridRef.current = grid;
        scene.add(grid);

        const ambientLight = new THREE.HemisphereLight(0xf7fbff, 0x7d7468, 2.2);
        const keyLight = new THREE.DirectionalLight(0xffffff, 3.5);
        const rimLight = new THREE.DirectionalLight(0xf18b67, 1.4);

        keyLight.position.set(4, 7, 5);
        rimLight.position.set(-5, 2, -4);
        scene.add(ambientLight);
        scene.add(keyLight);
        scene.add(rimLight);

        const controls = new OrbitControls(camera, renderer.domElement);
        controls.enableDamping = true;
        controls.dampingFactor = 0.07;
        controls.screenSpacePanning = true;

        const selectionController = new SelectionController({
            data,
            renderer,
            camera,
            controls,
            editMode: editModeRef.current,
            surfaceGeometry: geometry,
            meshRadius,
            mesh,
            wireframe,
            vertices,
            onSelectionChange: setSelection,
            onHoverSelectionChange: setHoverSelection,
        });
        selectionControllerRef.current = selectionController;

        const fit = () => {
            const sphere = geometry.boundingSphere;
            const box = geometry.boundingBox;

            if (!sphere || !box) {
                return;
            }

            const radius = Math.max(sphere.radius, 0.001);

            camera.near = radius / 100;
            camera.far = radius * 100;
            camera.updateProjectionMatrix();
            camera.position
                .copy(sphere.center)
                .add(new THREE.Vector3(radius * 1.8, radius * 1.05, radius * 2.5));

            controls.target.copy(sphere.center);
            controls.update();

            grid.position.set(sphere.center.x, box.min.y - radius * 0.05, sphere.center.z);
            grid.scale.setScalar(Math.max(radius / 5, 0.001));
        };

        fitRef.current = fit;
        fit();

        const resize = () => {
            const width = host.clientWidth;
            const height = host.clientHeight;

            renderer.setSize(width, height, false);
            renderer.getDrawingBufferSize(drawingBufferSize);
            selectionController.setSize(drawingBufferSize.x, drawingBufferSize.y);
            camera.aspect = width / Math.max(height, 1);
            camera.updateProjectionMatrix();
        };

        const observer = new ResizeObserver(resize);
        observer.observe(host);
        resize();

        let frame = 0;

        const render = () => {
            controls.update();
            renderer.render(scene, camera);
            frame = requestAnimationFrame(render);
        };

        render();

        return () => {
            cancelAnimationFrame(frame);
            observer.disconnect();
            selectionController.dispose();
            selectionMarkers.dispose();
            controls.dispose();
            geometry.dispose();
            material.dispose();
            wireframeGeometry.dispose();
            wireframeMaterial.dispose();
            verticesGeometry.dispose();
            verticesMaterial.dispose();
            renderer.dispose();
            renderer.domElement.remove();

            meshRef.current = null;
            wireframeRef.current = null;
            verticesRef.current = null;
            selectionControllerRef.current = null;
            selectionMarkersRef.current = null;
            gridRef.current = null;
            fitRef.current = null;
        };
    }, [data]);

    useEffect(() => {
        if (!data) {
            return;
        }

        const mesh = meshRef.current;

        if (mesh) {
            mesh.visible = view.mesh;
        }

        if (wireframeRef.current) {
            wireframeRef.current.visible = view.wireframe;
        }

        if (gridRef.current) {
            gridRef.current.visible = view.grid;
        }

        if (verticesRef.current) {
            verticesRef.current.visible = view.vertices;
        }

        selectionMarkersRef.current?.update({
            selection,
            hoverSelection,
            view,
        });
    }, [data, hoverSelection, selection, view]);

    const toggle = (key: keyof ViewState) => {
        setView((current) => ({
            ...current,
            [key]: !current[key],
        }));
    };

    const changeEditMode = (mode: EditMode) => {
        editModeRef.current = mode;
        selectionControllerRef.current?.setEditMode(mode);
        setEditMode(mode);
    };

    const count = (value?: number) => value?.toLocaleString('ja-JP') ?? '—';

    return (
        <main className="editor-shell">
            <header className="topbar">
                <div className="brand">
                    <span className="brand-mark" />
                    <span className="brand-name">HalfEdge Editor</span>
                </div>
                <span className="file-name">bunny6000.half</span>
                <nav className="mode-tabs" aria-label="編集対象">
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'object' ? 'active' : ''}`}
                        aria-pressed={editMode === 'object'}
                        onClick={() => changeEditMode('object')}
                    >
                        {EDIT_MODE_LABELS.object}
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'vertex' ? 'active' : ''}`}
                        aria-pressed={editMode === 'vertex'}
                        onClick={() => changeEditMode('vertex')}
                    >
                        {EDIT_MODE_LABELS.vertex}
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'edge' ? 'active' : ''}`}
                        aria-pressed={editMode === 'edge'}
                        onClick={() => changeEditMode('edge')}
                    >
                        {EDIT_MODE_LABELS.edge}
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'face' ? 'active' : ''}`}
                        aria-pressed={editMode === 'face'}
                        onClick={() => changeEditMode('face')}
                    >
                        {EDIT_MODE_LABELS.face}
                    </button>
                </nav>
                <div className="top-actions">
                    <button
                        className="tool-button"
                        onClick={() => fitRef.current?.()}
                    >
                        Frame selected
                    </button>
                </div>
            </header>

            <div className="workspace">
                <aside className="panel panel-left">
                    <section className="panel-section">
                        <h2 className="section-label">Scene</h2>
                        <div className="scene-item">
                            <span className="scene-dot" />
                            Stanford Bunny
                        </div>
                        <div className="tree-child">
                            Half-edge mesh · version {data?.version ?? '—'}
                        </div>
                    </section>

                    <section className="panel-section">
                        <h2 className="section-label">Topology</h2>
                        <dl className="stat-grid">
                            <dt>{ELEMENT_LABELS.vertex}</dt>
                            <dd>{count(data?.vertexCount)}</dd>
                            <dt>{ELEMENT_LABELS.edge}</dt>
                            <dd>{count(data?.edgeCount)}</dd>
                            <dt>{ELEMENT_LABELS.face}</dt>
                            <dd>{count(data?.faceCount)}</dd>
                        </dl>
                        <p className="format-note">
                            C++版 HalfEdgeLoader と同じversion 2バイナリを
                            ブラウザで直接読み込んでいます。
                        </p>
                    </section>
                </aside>

                <section
                    className="viewport"
                    ref={viewportRef}
                    aria-label="Stanford Bunny 3D viewport"
                >
                    {data && (
                        <div className="viewport-badge">
                            <i />
                            Loaded from native .half
                        </div>
                    )}
                    {!data && !error && (
                        <div className="loading-card">
                            <strong>Stanford Bunnyを読み込み中</strong>
                            <span>Half-edge topologyを解析しています</span>
                        </div>
                    )}
                    {error && (
                        <div className="loading-card error">
                            <strong>読み込みに失敗しました</strong>
                            <span>{error}</span>
                        </div>
                    )}
                    <div className="hint">
                        左ドラッグ 回転　·　右ドラッグ 移動　·　ホイール ズーム
                    </div>
                </section>

                <aside className="panel panel-right">
                    <section className="panel-section">
                        <h2 className="section-label">Display</h2>
                        <div className="toggle-row">
                            <span>{ELEMENT_LABELS.face}</span>
                            <button
                                aria-label={`${ELEMENT_LABELS.face}表示`}
                                className={`toggle ${view.mesh ? 'on' : ''}`}
                                onClick={() => toggle('mesh')}
                            />
                        </div>
                        <div className="toggle-row">
                            <span>{ELEMENT_LABELS.edge}</span>
                            <button
                                aria-label={`${ELEMENT_LABELS.edge}表示`}
                                className={`toggle ${view.wireframe ? 'on' : ''}`}
                                onClick={() => toggle('wireframe')}
                            />
                        </div>
                        <div className="toggle-row">
                            <span>{ELEMENT_LABELS.vertex}</span>
                            <button
                                aria-label={`${ELEMENT_LABELS.vertex}表示`}
                                className={`toggle ${view.vertices ? 'on' : ''}`}
                                onClick={() => toggle('vertices')}
                            />
                        </div>
                        <div className="toggle-row">
                            <span>Ground grid</span>
                            <button
                                aria-label="Grid表示"
                                className={`toggle ${view.grid ? 'on' : ''}`}
                                onClick={() => toggle('grid')}
                            />
                        </div>
                    </section>

                    <section className="panel-section">
                        <h2 className="section-label">Material</h2>
                        <div className="toggle-row">
                            <span>{ELEMENT_LABELS.face} color</span>
                            <span className="swatch" />
                        </div>
                        <div className="toggle-row">
                            <span>Shading</span>
                            <span>{ELEMENT_LABELS.vertex} normals</span>
                        </div>
                        <div className="toggle-row">
                            <span>Side</span>
                            <span>Double</span>
                        </div>
                    </section>

                    <section className="panel-section">
                        <h2 className="section-label">Selection</h2>
                        <div className="toggle-row" aria-live="polite">
                            <span>{getSelectionLabel(selection)}</span>
                            <span>
                                {selection === null ? 'None' : `#${selection.index}`}
                            </span>
                        </div>
                    </section>

                    <section className="panel-section">
                        <h2 className="section-label">Next</h2>
                        <p className="format-note">
                            {ELEMENT_LABELS.vertex} / {ELEMENT_LABELS.edge} / {ELEMENT_LABELS.face}をGPU ID pickingで
                            選択できます。
                        </p>
                    </section>
                </aside>
            </div>

            <footer className="statusbar">
                <span className="ready">● Renderer ready</span>
                <span>WebGL · Three.js</span>
                <span className="spacer" />
                <span>
                    {data
                        ? `${count(data.vertexCount)} ${ELEMENT_LABELS.vertex}`
                        : 'Loading geometry…'}
                </span>
            </footer>
        </main>
    );
}
