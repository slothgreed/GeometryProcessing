#ifndef FILE_UTILITY_H
#define FILE_UTILITY_H
#include <iostream>
#include <fstream>
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
	static String GetDirectoryPath(const String& filePath);
	static bool CheckExtension(const String& filePath, const String& ext);
	static Vector<String> Split(const String& str, char del);

private:

};

class FileWriter
{
public:
	FileWriter();
	~FileWriter();

	bool Open(const String& filePath, bool binary = false);
	void Write(const String& contents, bool endl = false);
	void WriteBinary(void* contents, Format format, bool endl = false);
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
	float ReadFloat();
	vec2 ReadVec2();
	Vector3 ReadVec3();
	vec4 ReadVec4();
	bool EndOfFile();
	void Close();
private:
	std::ifstream m_fileStream;
};
}
#endif FILE_UTILITY_H