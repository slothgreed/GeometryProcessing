#include "FileUtility.h"
#include <filesystem>
namespace KI
{
FileUtility::FileUtility()
{
}

FileUtility::~FileUtility()
{
}

bool FileUtility::Load(const String& filePath, String& contents)
{
	if (IsExist(filePath))
	{
		return false;
	}

	std::ifstream ifs(filePath);
	String line;
	while ((getline(ifs, line)))
	{
		contents += line + "\n";
	}

	return true;
}


bool FileUtility::Load(const String& filePath, Vector<String>& lines)
{
	if (IsExist(filePath) == false)
	{
		return false;
	}

	std::ifstream ifs(filePath);
	String line;
	while ((getline(ifs, line)))
	{
		lines.push_back(line);
	}

	return true;
}


bool FileUtility::IsExist(const String& filePath)
{
	std::ifstream fs(filePath);
	return fs.is_open();
}

String FileUtility::GetExtension(const String& filePath)
{
	int index = (int)filePath.find_last_of(".");
	return filePath.substr(index, filePath.size() - index);
}

void FileUtility::GetDirectoryPath(const String& filePath, String& directoryPath)
{
	int index = (int)filePath.find_last_of("\\");
	//directoryPath = filePath.substr(index, filePath.size() - index);
	directoryPath = filePath.substr(0, index + 1);
}
bool FileUtility::CheckExtension(const String& filePath, const String& ext)
{
	String extname = GetExtension(filePath);

	if (extname == ext)
	{
		return true;
	}
	else
	{
		return false;
	}
}

Vector<String> FileUtility::Split(const String& str, char del)
{
	int first = 0;
	int last = str.find_first_of(del);

	Vector<String> result;

	while (first < str.size()) {
		String subStr(str, first, last - first);

		result.push_back(subStr);

		first = last + 1;
		last = str.find_first_of(del, first);

		if (last == String::npos) {
			last = str.size();
		}
	}

	return result;
}

FileWriter::FileWriter()
{
}

FileWriter::~FileWriter()
{
	Close();
}

bool FileWriter::Open(const String& filePath, bool binary)
{
	//if (FileUtility::IsExist(filePath))
	//{
	//	return false;
	//}

	if (binary) {
		m_fileStream.open(filePath.data(), std::ios::out | std::ios::binary);
	}
	else {
		m_fileStream.open(filePath.data(), std::ios::out);
	}
	return true;
}

void FileWriter::Write(const String& contents, bool endl)
{
	assert(m_fileStream.is_open());
	m_fileStream << contents.data();
	if (endl) {
		m_fileStream << std::endl;
	}
}

void FileWriter::WriteBinary(void* contents, Format format, bool endl)
{
	assert(m_fileStream.is_open());
	switch (format)
	{
	case KI::Format::FLOAT:
		m_fileStream.write((char*)contents, sizeof(float));
		break;
	case KI::Format::VEC2:
		m_fileStream.write((char*)contents, sizeof(vec2));
		break;
	case KI::Format::VEC3:
		m_fileStream.write((char*)contents, sizeof(vec3));
		break;
	case KI::Format::VEC4:
		m_fileStream.write((char*)contents, sizeof(vec4));
		break;
	case KI::Format::INT:
		m_fileStream.write((char*)contents, sizeof(int));
		break;
	default:
		assert(0);
		break;
	}

}


void FileWriter::Close()
{
	if (m_fileStream.is_open()) {
		m_fileStream.close();
	}
}

FileReader::FileReader()
{
}

FileReader::~FileReader()
{
	Close();
}

bool FileReader::Open(const String& filePath, bool binary)
{
	//if (FileUtility::IsExist(filePath))
	//{
	//	return false;
	//}

	if (binary) {
		m_fileStream.open(filePath.data(), std::ios::in | std::ios::binary);
	}
	else {
		m_fileStream.open(filePath.data(), std::ios::in);
	}
	return true;
}

void FileReader::ReadAll(String& contents)
{
	contents = String(std::istreambuf_iterator<char>(m_fileStream), {});
}

bool FileReader::ReadLine(String& contents)
{
	if (getline(m_fileStream, contents)) {
		return true;
	}
	else {
		return false;
	}
}

int FileReader::ReadInt()
{
	assert(m_fileStream.is_open());
	int data = 0;
	m_fileStream.read((char*)&data, sizeof(int));
	return data;
}
float FileReader::ReadFloat()
{
	assert(m_fileStream.is_open());
	float data = 0;
	m_fileStream.read((char*)&data, sizeof(float));
	return data;
}
vec2 FileReader::ReadVec2()
{
	assert(m_fileStream.is_open());
	vec2 data;
	m_fileStream.read((char*)&data, sizeof(vec2));
	return data;
}
vec3 FileReader::ReadVec3()
{
	assert(m_fileStream.is_open());
	vec3 data;
	m_fileStream.read((char*)&data, sizeof(vec3));
	return data;
}
vec4 FileReader::ReadVec4()
{
	assert(m_fileStream.is_open());
	vec4 data;
	m_fileStream.read((char*)&data, sizeof(vec4));
	return data;
}



bool FileReader::EndOfFile()
{
	return m_fileStream.eof();
}
void FileReader::Close()
{
	if (m_fileStream.is_open()) {
		m_fileStream.close();
	}
}
}