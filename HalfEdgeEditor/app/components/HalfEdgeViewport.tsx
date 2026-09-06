'use client';

import { useEffect, useRef, useState } from 'react';
import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';
import { parseHalfEdgeFile, type HalfEdgeMeshData } from '../lib/halfEdgeLoader';

type ViewState = {
    mesh: boolean;
    wireframe: boolean;
    grid: boolean;
};

type EditMode = 'object' | 'vertex' | 'edge' | 'face';

export default function HalfEdgeViewport() {
    const viewportRef = useRef<HTMLDivElement>(null);
    const fitRef = useRef<(() => void) | null>(null);
    const meshRef = useRef<THREE.Mesh | null>(null);
    const verticesRef = useRef<THREE.Points | null>(null);
    const gridRef = useRef<THREE.GridHelper | null>(null);

    const [data, setData] = useState<HalfEdgeMeshData | null>(null);
    const [error, setError] = useState<string | null>(null);
    const [editMode, setEditMode] = useState<EditMode>('object');
    const [view, setView] = useState<ViewState>({
        mesh: true,
        wireframe: false,
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
        });

        const mesh = new THREE.Mesh(geometry, material);
        meshRef.current = mesh;
        scene.add(mesh);

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
        vertices.renderOrder = 1;
        verticesRef.current = vertices;
        scene.add(vertices);

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
            controls.dispose();
            geometry.dispose();
            material.dispose();
            verticesGeometry.dispose();
            verticesMaterial.dispose();
            renderer.dispose();
            renderer.domElement.remove();

            meshRef.current = null;
            verticesRef.current = null;
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
            const material = mesh.material as THREE.MeshStandardMaterial;

            mesh.visible = view.mesh;
            material.wireframe = view.wireframe;
            material.needsUpdate = true;
        }

        if (gridRef.current) {
            gridRef.current.visible = view.grid;
        }

        if (verticesRef.current) {
            verticesRef.current.visible = editMode === 'vertex';
        }
    }, [data, editMode, view]);

    const toggle = (key: keyof ViewState) => {
        setView((current) => ({
            ...current,
            [key]: !current[key],
        }));
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
                        onClick={() => setEditMode('object')}
                    >
                        Object
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'vertex' ? 'active' : ''}`}
                        aria-pressed={editMode === 'vertex'}
                        onClick={() => setEditMode('vertex')}
                    >
                        Vertex
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'edge' ? 'active' : ''}`}
                        aria-pressed={editMode === 'edge'}
                        onClick={() => setEditMode('edge')}
                    >
                        Edge
                    </button>
                    <button
                        type="button"
                        className={`mode-tab ${editMode === 'face' ? 'active' : ''}`}
                        aria-pressed={editMode === 'face'}
                        onClick={() => setEditMode('face')}
                    >
                        Face
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
                            <dt>Vertices</dt>
                            <dd>{count(data?.vertexCount)}</dd>
                            <dt>Half edges</dt>
                            <dd>{count(data?.halfEdgeCount)}</dd>
                            <dt>Faces</dt>
                            <dd>{count(data?.faceCount)}</dd>
                            <dt>Triangles</dt>
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
                            <span>Surface</span>
                            <button
                                aria-label="Surface表示"
                                className={`toggle ${view.mesh ? 'on' : ''}`}
                                onClick={() => toggle('mesh')}
                            />
                        </div>
                        <div className="toggle-row">
                            <span>Wireframe</span>
                            <button
                                aria-label="Wireframe表示"
                                className={`toggle ${view.wireframe ? 'on' : ''}`}
                                onClick={() => toggle('wireframe')}
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
                            <span>Surface color</span>
                            <span className="swatch" />
                        </div>
                        <div className="toggle-row">
                            <span>Shading</span>
                            <span>Vertex normals</span>
                        </div>
                        <div className="toggle-row">
                            <span>Side</span>
                            <span>Double</span>
                        </div>
                    </section>

                    <section className="panel-section">
                        <h2 className="section-label">Next</h2>
                        <p className="format-note">
                            Vertex / Edge / Face選択とTransformControlsを
                            次の編集スライスとして追加できます。
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
                        ? `${count(data.vertexCount)} vertices`
                        : 'Loading geometry…'}
                </span>
            </footer>
        </main>
    );
}
