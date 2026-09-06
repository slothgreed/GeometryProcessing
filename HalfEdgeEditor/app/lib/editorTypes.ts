export type ViewState = {
    mesh: boolean;
    wireframe: boolean;
    vertices: boolean;
    grid: boolean;
};

export type EditMode = 'object' | 'vertex' | 'edge' | 'face';

export type Selection = {
    type: Exclude<EditMode, 'object'>;
    index: number;
};
