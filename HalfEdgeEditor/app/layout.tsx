import type { Metadata } from 'next';
import './globals.css';

export const metadata: Metadata = {
  metadataBase: new URL('https://halfedge-editor.stmndrm.chatgpt.site'),
  title: 'HalfEdge Editor',
  description: 'A focused browser workspace for inspecting and editing half-edge meshes.',
  openGraph: {
    title: 'HalfEdge Editor',
    description: 'Inspect. Edit. Understand topology.',
    images: ['/og.png'],
  },
  twitter: {
    card: 'summary_large_image',
    title: 'HalfEdge Editor',
    description: 'Inspect. Edit. Understand topology.',
    images: ['/og.png'],
  },
};

export default function RootLayout({ children }: Readonly<{ children: React.ReactNode }>) {
  return <html lang="ja"><body>{children}</body></html>;
}
