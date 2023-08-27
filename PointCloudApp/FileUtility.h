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

	static bool IsExist(const string& filePath);
	static bool Load(const string& filePath, string& contents);
	static bool Load(const string& filePath, vector<string>& contents);
	static string GetExtension(const string& filePath);
	static void GetDirectoryPath(const string& filePath, string& directoryPath);
	static bool CheckExtension(const string& filePath, const string& ext);
	static std::vector<std::string> Split(const std::string& str, char del);

private:

};

class FileWriter
{
public:
	FileWriter();
	~FileWriter();

	bool Open(const std::string& filePath, bool binary = false);
	void Write(const std::string& contents, bool endl = false);
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

	bool Open(const std::string& filePath, bool binary = false);
	bool ReadLine(std::string& contents);
	void ReadAll(std::string& contents);
	int ReadInt();
	float ReadFloat();
	vec2 ReadVec2();
	vec3 ReadVec3();
	vec4 ReadVec4();
	bool EndOfFile();
	void Close();
private:
	std::ifstream m_fileStream;
};
}
#endif FILE_UTILITY_H