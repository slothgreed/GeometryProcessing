export type ViewState = {
    mesh: boolean;
    wireframe: boolean;
    vertices: boolean;
    grid: boolean;
};

export type EditMode = 'any' | 'vertex' | 'edge' | 'face';

export type Selection = {
    type: Exclude<EditMode, 'any'>;
    index: number;
};
