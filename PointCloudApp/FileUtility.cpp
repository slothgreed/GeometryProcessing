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

String FileUtility::GetDirectoryPath(const String& filePath)
{
	int index = (int)filePath.find_last_of("\\");
	//directoryPath = filePath.substr(index, filePath.size() - index);
	return filePath.substr(0, index + 1);
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
	return StringUtility::Split(str, del);
}

Vector<String> StringUtility::Split(const String& str, char del)
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

bool StringUtility::Contains(const String& str, const String& target)
{
	return str.find(target) != std::string::npos;
}

std::pair<String, String> StringUtility::SplitAtFirst(const String& str, char delimiter)
{
	size_t pos = str.find(delimiter);
	if (pos == std::string::npos) {
		return std::pair<String, String>(); // ��؂蕶����������Ȃ������ꍇ
	}

	auto left = str.substr(0, pos);         // ��؂蕶���̑O�̕���
	auto right = str.substr(pos + 1);      // ��؂蕶���̌�̕���
	return std::make_pair(left, right);
}

String StringUtility::TrimWhiteSpace(const String& str)
{
	size_t start = str.find_first_not_of(" \t\n\r\f\v"); // �󔒂łȂ��ŏ��̈ʒu
	if (start == std::string::npos) return ""; // ���ׂċ󔒂Ȃ�󕶎���Ԃ�

	size_t end = str.find_last_not_of(" \t\n\r\f\v"); // �󔒂łȂ��Ō�̈ʒu
	return str.substr(start, end - start + 1);
}

String StringUtility::Remove(const String& str, char del)
{
	// str����del���폜�����V������������쐬
	String result = str;

	// del���폜�istd::remove�ŗv�f���ړ����Aerase�Ŏ��ۂɍ폜�j
	result.erase(std::remove(result.begin(), result.end(), del), result.end());

	return result;
}

String StringUtility::RemoveFirst(const String& str, int num)
{
	if (num <= 0 || str.size() <= static_cast<size_t>(num)) {
		return str;
	}

	return std::string(str.c_str() + num, str.size() - num);
}

String StringUtility::RemoveLast(const String& str, int num)
{
	if (num <= 0 || str.size() <= static_cast<size_t>(num)) {
		return str;
	}

	return std::string(str.c_str(), str.size() - num);
}

String StringUtility::After(const String& str, char del)
{
	size_t pos = str.find(del);
	if (pos != std::string::npos) {
		return str.substr(pos + 1);  // del�̎��̕�������Ō�܂�
	}
	return "";  // del��������Ȃ������ꍇ�͋󕶎����Ԃ�
}

String StringUtility::After(const String& str, int pos)
{
	return str.substr(pos);
}

int StringUtility::FindFirst(const String& str, char del)
{
	size_t pos = str.find(del);
	if (pos != std::string::npos) {
		return pos;  // del�̎��̕�������Ō�܂�
	}

	return -1;
}

String StringUtility::Before(const String& str, int pos)
{
	return str.substr(0, pos);  // delimiter���O�̕���
}

String StringUtility::Before(const String& str, char del)
{
	size_t pos = str.find(del);
	if (pos != std::string::npos) {
		return str.substr(0, pos);  // delimiter���O�̕���
	}
	return str;  // delimiter��������Ȃ������ꍇ�͑S�̂�Ԃ�
}

int StringUtility::ToInt(const String& str)
{
	return std::atoi(str.data());
}

float StringUtility::ToFloat(const String& str)
{
	return std::atof(str.data());
}

String StringUtility::ToString(int value)
{
	return std::to_string(value);
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
		m_fileStream.write((char*)contents, sizeof(Vector3));
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
Vector3 FileReader::ReadVec3()
{
	assert(m_fileStream.is_open());
	Vector3 data;
	m_fileStream.read((char*)&data, sizeof(Vector3));
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