#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H
#include <iostream>
#include "Windows.h"
#include <fstream>
#include <span>
namespace KI
{
enum class Format
{
	INT,
	FLOAT,
	VEC2,
	VEC3,
	VEC4
};
class FileUtility
{
public:
	FileUtility();
	~FileUtility();

	static bool IsExist(const String& filePath);
	static bool Load(const String& filePath, String& contents);
	static bool Load(const String& filePath, Vector<String>& contents);
	static String GetExtension(const String& filePath);
	static String RemoveExtension(const String& filePath);
	static String GetDirectoryPath(const String& filePath);
	static String GetFileName(const String& filePath);
	static bool CheckExtension(const String& filePath, const String& ext);
	static Vector<String> Split(const String& str, char del);
	static int FileNum(const String& directory);
	static Vector<String> CollectFile(const String& directory, const String& ext);
	static String GetExecutableDirectory();
	static String GetCacheDirectory();
	static String CreateCacheDirectory();
private:

};

class StringUtility
{
public:
	StringUtility() {};
	~StringUtility() {};

	static Vector<String> Split(const String& str, char del);
	static String Remove(const String& str, char del);
	static String RemoveFirst(const String& str, int num);
	static String RemoveLast(const String& str, int num);
	static String Before(const String& str, char del);
	static String Before(const String& str, int pos);
	static String After(const String& str, char del);
	static String After(const String& str, int pos);
	static int FindFirst(const String& str, char del);
	static int ToInt(const String& str);
	static float ToFloat(const String& str);
	static String ToString(int value);
	static String ToString(const Vector3& str);
	static std::pair<String, String> SplitAtFirst(const String& str, char delimiter);
	static String TrimWhiteSpace(const String& str);
	static bool Contains(const String& str, const String& target);
	static bool Equal(const String& str, const String& target);

private:

};


class FileWriter
{
public:
	FileWriter();
	~FileWriter();

	bool Open(const String& filePath, bool binary = false);
	void Write(const String& contents, bool endl = false);
	void Write(const Vector<float>& contents, bool endl = false);
	void WriteBinary(void* contents, Format format, bool endl = false);
	void WriteBinary(int contents);
	void WriteBinary(const Vector3& contents);
	void WriteBinary(const Vector<float>& contents);
	void Close();
private:
	std::ofstream m_fileStream;
};

class FileReader
{
public:
	FileReader();
	~FileReader();

	bool Open(const String& filePath, bool binary = false);
	bool ReadLine(String& contents);
	void ReadAll(String& contents);
	int ReadInt();
	Vector<int> ReadInt(int num);
	float ReadFloat();
	vec2 ReadVec2();
	Vector3 ReadVec3();
	vec4 ReadVec4();
	template <typename T>
	Vector<T> ReadVector(int num)
	{
		static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
		static_assert(std::is_standard_layout<T>::value, "T must have standard layout");

		Vector<T> data(num);
		m_fileStream.read(reinterpret_cast<char*>(data.data()), sizeof(T) * num);
		return data;
	}

	template <typename T>
	bool ReadVector(T* data, int num)
	{
		static_assert(std::is_trivially_copyable<T>::value, "T must be trivially copyable");
		static_assert(std::is_standard_layout<T>::value, "T must have standard layout");

		m_fileStream.read(reinterpret_cast<char*>(data), sizeof(T) * num);
		return m_fileStream.good();
	}

	bool EndOfFile();
	void Close();
private:
	std::ifstream m_fileStream;
};

class MemoryMappedFile
{
public:
	MemoryMappedFile();
	~MemoryMappedFile();

	void Open(const String& path);
	void Close();

	const char* Data() const { return m_data; }
	std::string_view Text() const { return std::string_view(m_data, m_size); }
private:
	HANDLE m_file = INVALID_HANDLE_VALUE;
	HANDLE m_mapping = nullptr;

	const char* m_data = nullptr;

	size_t m_size = 0;
};

class LineReader
{
public:
	class Iterator
	{
	public:
		using value_type = std::string_view;
		Iterator() :m_finished(true) {}
		explicit Iterator(std::string_view text)
			: m_text(text)
		{
			Next();
		}
		value_type operator*() const { return m_line; }
		Iterator& operator++() { Next(); return *this; }
		bool operator==(const Iterator& other) const
		{
			return m_finished == other.m_finished;
		}

		bool operator!=(const Iterator& other) const { return !(*this == other); }

	private:

		void Next()
		{
			if (m_text.empty()) {
				m_finished = true;
				m_line = {};
				return;
			}

			size_t pos = m_text.find('\n');

			if (pos == std::string_view::npos) {
				m_line = m_text;
				m_text = {};
			} else {
				m_line = m_text.substr(0, pos);
				m_text.remove_prefix(pos + 1);
			}

			// Windows(CRLF)‘Î‰ž
			if (!m_line.empty() && m_line.back() == '\r')
				m_line.remove_suffix(1);
		}

		std::string_view m_text;
		std::string_view m_line;
		bool m_finished = false;
	};

	explicit LineReader(std::string_view text) : m_text(text) {}
	Iterator begin() const { return Iterator(m_text); }
	Iterator end() const { return Iterator(); }

private:
	std::string_view m_text;
};
}
#endif FILE_UTILITY_H