#ifndef IPRIMITIVE_H
#define IPRIMITIVE_H
class BDB;
struct Vertex
{
	glm::vec3 m_position;
	glm::vec3 m_normal;
};

class Primitive
{
public:
	Primitive();
	virtual ~Primitive() {};

	enum StoreType
	{
		Array,
		Interleave
	};
public:
	BDB CreateBDB() const;
	GLuint GetDrawType();
	void CalcNormal(); // 適当コード
	bool IsInterleave() { return m_storeType == StoreType::Interleave; }
	StoreType GetStoreType() { return m_storeType; }
	void SetPosition(std::vector<glm::vec3>&& value) { m_position = std::move(value); }
	std::vector<glm::vec3>& Position() { return m_position; };
	void SetNormal(std::vector<glm::vec3>&& value) { m_normal = std::move(value); }
	std::vector<glm::vec3>& Normal() { return m_normal; };
	void SetColor(std::vector<glm::vec3>&& value) { m_color = std::move(value); }
	std::vector<glm::vec3>& Color() { return m_color; };

	std::vector<Vertex>& GetVertex() { return m_vertex; };
	std::vector<int>& Index() { return m_index; };
	void Multi(const mat4x4& matrix);
	void Convert(StoreType type);
	std::shared_ptr<Primitive> Clone();
	int GetTriangleNum();
	bool NeedUpdate() { return m_update; }
	void ClearUpdate() { m_update = false; }
	void Update() { m_update = true; }
protected:
	bool m_update;
	GLuint m_drawType;
	StoreType m_storeType;
	std::vector<glm::vec3> m_color;
	std::vector<glm::vec3> m_position;
	std::vector<glm::vec3> m_normal;
	std::vector<Vertex> m_vertex;

	std::vector<int> m_index;
	
};

#endif
