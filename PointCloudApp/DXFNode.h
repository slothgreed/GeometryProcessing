#ifndef DFX_NODE_H
#define DFX_NODE_H

#include "RenderNode.h"
namespace KI
{


constexpr int DXF_UNKNOWN = -1;
constexpr int DXF_ENTITY = 0;
constexpr int DXF_NAME = 2;
constexpr int DXF_DESCRIPTION = 3;
constexpr int DXF_LAYER_NAME = 8;
constexpr int DXF_SYSTEM_VARIABLE =9;
constexpr int DXF_X0 = 10;
constexpr int DXF_Y0 = 20;
constexpr int DXF_Z0 = 30;
constexpr int DXF_X1 = 11;
constexpr int DXF_Y1 = 21;
constexpr int DXF_Z1 = 31;
constexpr int DXF_RADIUS = 40;
constexpr int DXF_COLOR = 62;
constexpr int DXF_NEXT_ENTITY = 66;
constexpr int DXF_FLAG = 70;
constexpr int DXF_LINE_TYPE = 6;
constexpr int DXF_BEGIN_ANGLE = 50;
constexpr int DXF_END_ANGLE = 51;


inline bool IsX(int code) { return (int)code >= 10 && (int)code <= 18; }
inline bool IsY(int code) { return (int)code >= 20 && (int)code <= 28; }
inline bool IsZ(int code) { return (int)code >= 30 && (int)code <= 38; }

enum class DXFEntityType
{
	DXF_SECTION,
	DXF_ENTITIES,
	DXF_LINE,
	DXF_ENDSEC,
	DXF_EOF
};

enum class DXFLineType
{
	Unknown,
	Continuous,
	Dashed,
	Center,
};

inline DXFLineType ToDXFLineType(const String& s)
{
	if (s == "CONTINUOUS") return DXFLineType::Continuous;
	return DXFLineType::Unknown;
}

enum class DXFRecordType
{
	Unknown,
	Section,
	Line,
	LType,
	Text,
	Vertex,
	Arc,
	Point,
	Polyline,
	EndSec,
	Eof
};

enum class DXFSectionType
{
	Unknown,
	Header,
	Tables,
	Blocks,
	Entities
};


inline DXFRecordType ToDXFRecordType(const String& s)
{
	if (s == "SECTION")	return DXFRecordType::Section;
	if (s == "LTYPE")	return DXFRecordType::LType;
	if (s == "LINE")    return DXFRecordType::Line;
	if (s == "TEXT")    return DXFRecordType::Text;
	if (s == "POLYLINE")  return DXFRecordType::Polyline;
	if (s == "VERTEX")  return DXFRecordType::Vertex;
	if (s == "ARC")  return DXFRecordType::Arc;
	if (s == "POINT")  return DXFRecordType::Point;
	if (s == "ENDSEC")  return DXFRecordType::EndSec;
	if (s == "EOF")     return DXFRecordType::Eof;
	return DXFRecordType::Unknown;
}

inline DXFSectionType ToDXFSectionType(const String& s)
{
	if (s == "ENTITIES") return DXFSectionType::Entities;
	if (s == "HEADER")   return DXFSectionType::Header;
	if (s == "TABLES")   return DXFSectionType::Tables;
	if (s == "BLOCKS")   return DXFSectionType::Blocks;
	return DXFSectionType::Unknown;
}

struct DXFLType
{
	String name;
	Vector<float> pattern;
};

struct DXFLine
{
	String layerName;
	Vector3 begin;
	Vector3 end;
};

struct DXFText
{

};

struct DXFArc
{
	String layerName;
	Vector3 center;
	float radius = 0.0f;
	float beginAngle = 0.0f;
	float endAngle = 0.0f;
};

struct DXFPoint
{
	String layerName;
	Vector3 point;
};

struct DXFVertex
{
	String layerName;
	Vector3 point;
};
struct DXFPolyline
{
	String layerName;
	DXFLineType lineType;
	int color = 0;
	Vector3 point;
	int flag = 0;
	Vector<DXFVertex> vertexs;
};


struct DXFStruct
{
public:
	DXFStruct() {};
	~DXFStruct() {};

	Vector<DXFPoint> points;
	Vector<DXFArc> arcs;
	Vector<DXFLine> lines;
	Vector<DXFPolyline> polylines;
	Vector<DXFText> texts;
	Vector<DXFLType> ltype;

	Vector<std::pair<int, String>> codeValue;
private:
	
};

class DXFLoader
{
public:
	DXFLoader() {};
	~DXFLoader() {};

	static DXFStruct* Load(const String& name);
private:
	static DXFArc ParseArc(const Vector<String>& lines, size_t& index);
	static DXFPoint ParsePoint(const Vector<String>& lines, size_t& index);
	static DXFText ParseText(const Vector<String>& lines, size_t& index);
	static DXFLType ParseLType(const Vector<String>& lines, size_t& index);
	static DXFLine ParseLine(const Vector<String>& lines, size_t& index);
	static DXFPolyline ParsePolyLine(const Vector<String>& lines, size_t& index);
	static DXFVertex ParseVertex(const Vector<String>& lines, size_t& index);
};


class DXFNode : public RenderNode
{
public:
	DXFNode(const String& name, const Shared<DXFStruct>& pDXF);
	~DXFNode() {};
	virtual void DrawNode(const DrawContext& context);
	virtual void ShowUI(UIContext& ui);

private:
	void BuildGLBuffer();
	Shared<DXFStruct> m_pDXF;
	Unique<GLBuffer> m_pLines;
	Vector<Unique<GLBuffer>> m_pPolylines;
	Vector<Unique<GLBuffer>> m_pArcs;
	Unique<GLBuffer> m_pPoints;

};
}

#endif DFX_NODE_H