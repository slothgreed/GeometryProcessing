import type { EditMode, Selection } from './editorTypes';

export const ELEMENT_LABELS = {
    vertex: 'Vertex',
    edge: 'Edge',
    face: 'Face',
} as const;

export const EDIT_MODE_LABELS: Record<EditMode, string> = {
    any: 'Any',
    vertex: ELEMENT_LABELS.vertex,
    edge: ELEMENT_LABELS.edge,
    face: ELEMENT_LABELS.face,
};

export function getSelectionLabel(selection: Selection | null) {
    return selection ? ELEMENT_LABELS[selection.type] : 'Selection';
}
