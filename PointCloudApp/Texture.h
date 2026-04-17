#ifndef TEXTURE_H
#define	TEXTURE_H
#include "GLTypes.h"
namespace KI
{

class Neighbor8
{
public:
	Neighbor8() {};
	~Neighbor8() {};

	static Neighbor8 Create()
	{
		Vector<Vector2i> v(8);
		v[0] = Vector2i(0, 1);	v[1] = Vector2i(1, 1);	v[2] = Vector2i(1, 0);
		v[3] = Vector2i(1, -1); v[4] = Vector2i(0, -1);
		v[5] = Vector2i(-1, -1); v[6] = Vector2i(-1, 0); v[7] = Vector2i(-1, 1);
		
		Neighbor8 n;
		n.m_value = std::move(v);
		return n;
	}

	static int Size() { return 8; }
	const Vector2i& operator[](int i) const { return m_value[i]; }
	static int Opposite(int i) { return (i + 4) & 7; }
	static int Prev(int i) { return (i + 7) & 7; }
	static int Next(int i) { return (i + 1) & 7; }
	static int Index(int i) { return i & 7; }
private:
	Vector<Vector2i> m_value;
};

struct PixelData
{
	struct Iterator
	{
		Iterator(const PixelData* pixel, int index)
			: m_pixel(pixel)
			, m_index(index)
		{
		}

		bool operator!=(const Iterator& other) const { return m_index != other.m_index; }
		Iterator& operator++() { ++m_index; return *this; }
		Vector4 operator*() const
		{
			int c = m_pixel->component;
			const unsigned char* p = m_pixel->data + m_index * c;
			Vector4 v{ 0, 0, 0, 255 };
			if (c > 0) v.x = p[0];
			if (c > 1) v.y = p[1];
			if (c > 2) v.z = p[2];
			if (c > 3) v.w = p[3];
			return v;
		}

	private:
		const PixelData* m_pixel = nullptr;
		int m_index = 0;
	};


	PixelData() {};
	~PixelData() { Free(); };

	Iterator begin() const { return PixelData::Iterator(this, 0); }
	Iterator end() const { return PixelData::Iterator(this, width * height); }

	bool IsIn(const Vector2i& xy) const { return IsIn(xy.x, xy.y); }

	bool IsIn(int x, int y) const
	{
		return
			0 <= x && x < width &&
			0 <= y && y < height;
	}
	Vector4 Get(const Vector2i& xy) const
	{
		return Get(xy.x, xy.y);
	}
	Vector4 Get(int x, int y) const
	{
		if (data == nullptr) { return Vector4(0); }
		if (!IsIn(x, y)) { return Vector4(0); }
		unsigned char* p = &data[(y * width + x) * component];
		Vector4 v{ 0, 0, 0, 255 };
		if (component > 0) v.x = p[0];
		if (component > 1) v.y = p[1];
		if (component > 2) v.z = p[2];
		if (component > 3) v.w = p[3];
		return v;
	}

	void Free()
	{
		if (m_allocate) { delete[] data; } m_allocate = false;
	}
	void Allocate(int x, int y, int _component)
	{
		Free();
		data = new unsigned char[x * y * _component];
		std::fill(data, data + x * y * _component, 0);
		width = x;
		height = y;
		component = _component;
		m_allocate = true;
	}
	void Set(int x, int y, const Vector4& value)
	{
		if (component == 4) {
			data[(y * width + x) * component + 0] = value.x;
			data[(y * width + x) * component + 1] = value.y;
			data[(y * width + x) * component + 2] = value.z;
			data[(y * width + x) * component + 3] = value.a;
		} else {
			assert(0);
		}
	}
	
	Vector2i IndexToXY(int index) const
	{
		return Vector2i(index % width, index / width);
	}
	int width = 0;
	int height = 0;
	int component = 0;
	unsigned char* data = nullptr;
	bool m_allocate = false;
};
class Texture
{
public:
	

	class Format
	{
	public:
		GLenum target = GL_TEXTURE_2D;
		GLint level = 0;
		GLint internalformat = GL_RGBA;
		GLsizei width = 1;
		GLsizei height = 1;
		GLsizei depth = 1;
		GLint border = 0;
		GLenum format = GL_RGBA;
		GLenum type = GL_UNSIGNED_BYTE;
		Format() {}
		~Format() {}

		static Format Create(int width, int height)
		{
			Format format;
			format.width = width;
			format.height = height;
			return format;
		}

		bool operator==(const Format& rhs) const
		{
			return target == rhs.target &&
				level == rhs.level &&
				internalformat == rhs.internalformat &&
				width == rhs.width &&
				height == rhs.height &&
				border == rhs.border &&
				format == rhs.format &&
				type == rhs.type;
		}

		bool operator!=(const Format& rhs) const
		{
			return !(*this == rhs);
		}
	};

	class Sampler
	{
	public:
		enum WRAP
		{
			REPEAT = 0x2901,
		};

		enum FILTER
		{
			LINEAR = 0x2601,
			NEAREST = 0x2600
		};

		Sampler()
			: wrap(WRAP::REPEAT)
			, filter(FILTER::NEAREST)
			, mipmap(false)
		{
		}

		Sampler(FILTER _f)
			: wrap(WRAP::REPEAT)
			, filter(_f)
			, mipmap(false)
		{
		}

		void UseMipmap()
		{
			mipmap = true;
		}
		bool mipmap;
		WRAP wrap;
		FILTER filter;
	};

	Texture();
	~Texture();
	void Bind();
	void Delete();
	virtual TEXTURE_TYPE Type() const = 0;
	GLuint Handle() const { return m_handle; }
	Vector2i Size() const { return Vector2i(m_format.width, m_format.height); }
	const Format& GetFormat() const { return m_format; }
	void Set(const Format& format, unsigned char* data);
	void GetPixel(std::vector<unsigned char>& data);
	void UseMipmap() { m_sampler.UseMipmap(); }
	static int CalcMipmapLevel(const Vector2i& resolute);
	PixelData GetPixelData();
	static Vector2i CalcMipmapResolute(const Vector2i& resolute, int mipLevel);
protected:
	bool m_genMip;
	Sampler m_sampler;
	Format m_format;
	GLuint m_handle;
	std::vector<unsigned char> m_data;

};

class TextureBuffer : public Texture
{
public:
	TextureBuffer();
	~TextureBuffer();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_BUFFER; }
	void Bind(int matrixId);
private:
};

class Texture2D : public Texture
{
public:
	Texture2D();
	Texture2D(const Format& format, const Sampler& sampler);

	~Texture2D();

	virtual TEXTURE_TYPE Type() const { return TEXTURE_2D; }
	static Texture2D* Create(const Vector2i& resolute, const Vector4& color);
	static Texture2D* Create(const Texture2D& texture);
	void Build(int width, int height);
	void Build(int width, int height, unsigned char* data);
	void Build(int width, int height, const Sampler& sampler, unsigned char* data);
	void SetData(Vector<unsigned char>&& data) { m_data = data; }
	void BindSampler(const Sampler& sampler);
	static Texture::Format CreateRGBA(int width, int height);
	void Clear(int value);
	void ClearMaxValue();
	void Resize(int width, int height);
	void Copy(const Texture2D& texture);
private:
};

class Texture3D : public Texture
{
public:
	Texture3D();
	~Texture3D();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_3D; }
	void Build(const Vector3i& size, Vector4* data);
	void Build(const Vector3i& size, unsigned short* data);

private:

};


class CubemapTexture : public Texture
{
public:
	CubemapTexture();
	~CubemapTexture();
	virtual TEXTURE_TYPE Type() const { return TEXTURE_CUBE_MAP; }

	void Build(const Vector2i& resolute, bool mipmap = true);
	void Build(const Vector<String>& path, bool mipmap = true);
	void BuildArray(const Vector2i& resolute, bool mipmap = true);

	static CubemapTexture* ConvertCubemap(const CubemapTexture* arr);

private:

};

}


#endif TEXTURE_H