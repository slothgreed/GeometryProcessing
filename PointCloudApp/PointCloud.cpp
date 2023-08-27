#include "PointCloud.h"
#include "FileUtility.h"
#include "Random.h"
#include <iostream>
#include <string>
PointCloud::PointCloud()
{
}

PointCloud::~PointCloud()
{
}

void PointCloud::LoadPCD(const std::string& name)
{
	KI::FileReader reader;
	reader.Open(name);
	std::string contents;
	int line = -1;
	int index = 0;
	while (reader.ReadLine(contents))
	{
		line++;
		if (line == 2)
		{
			auto split = KI::FileUtility::Split(contents, ' ');
			split.size();
		}
		if (line == 9)
		{
			auto split = KI::FileUtility::Split(contents, ' ');
			int size = std::stoi(split[1]);
			m_position.resize(size);
			m_normal.resize(size);
			m_color.resize(size);
			m_curvature.resize(size);
		}
		if (line < 11) { continue; }

		auto data = KI::FileUtility::Split(contents, ' ');

		m_position[index].x = std::stof(data[0]);
		m_position[index].y = std::stof(data[1]);
		m_position[index].z = std::stof(data[2]);

		m_normal[index].x = std::stof(data[0]);
		m_normal[index].y = std::stof(data[1]);
		m_normal[index].z = std::stof(data[2]);


		//m_normal[index].x = std::stof(data[3]);
		//m_normal[index].y = std::stof(data[4]);
		//m_normal[index].z = std::stof(data[5]);

		//m_curvature[index] = std::stof(data[6]);
		index++;
	}
}

void PointCloud::LoadXYZ(const std::string& name)
{
	KI::FileReader reader;
	reader.Open(name);
	std::string contents;
	int line = -1;
	int index = 0;
	while (reader.ReadLine(contents))
	{
		auto line = KI::FileUtility::Split(contents, ' ');
		assert(line.size() == 6);
		
		m_position.push_back(vec3(std::stof(line[0]), std::stof(line[1]), std::stof(line[2])));
		m_normal.push_back(vec3(std::stof(line[3]), std::stof(line[4]), std::stof(line[5])));
	}
}

PointCloud* PointCloud::Load(const std::string& name)
{
	auto pInstance = new PointCloud();
	auto ext = KI::FileUtility::GetExtension(name);
	pInstance->m_filePath = name;
	auto binPath = name + ".ki_bin";
	bool loadBin = false;
	if (ext == "ki_bin") {
		pInstance->LoadBin(name);
		loadBin = true;
	} else if (KI::FileUtility::IsExist(binPath)) {
		pInstance->LoadBin(binPath);
		loadBin = true;
	} else if (ext == "pcd") {
		pInstance->LoadPCD(name);
	} else {
		pInstance->LoadXYZ(name);
	}

	if (!loadBin) {
		pInstance->OutputBinary(binPath);
	}

	pInstance->m_index.resize(pInstance->m_position.size());
	for (int i = 0; i < pInstance->m_position.size(); i++) {
		pInstance->m_index[i] = i;
	}
	return pInstance;
}

PointCloud* PointCloud::Create2D(int positionNum, const glm::vec2& min, const glm::vec2& max)
{
	auto pInstance = new PointCloud();
	pInstance->m_position.resize(positionNum);
	pInstance->m_normal.resize(positionNum, vec3(0));	// FileIOópÇ…çÏÇ¡ÇƒÇ¢ÇÈÅB
	for (int i = 0; i < positionNum; i++) {
		pInstance->m_position[i].x = Gaccho::rnd(min.x, max.x);
		pInstance->m_position[i].y = Gaccho::rnd(min.y, max.y);
		pInstance->m_position[i].z = 0.0f;
	}

	return pInstance;
}
void PointCloud::LoadBin(const std::string& name)
{
	KI::FileReader reader;
	reader.Open(name, true);
	int num = reader.ReadInt();
	int normComp = reader.ReadInt();
	int colComp = reader.ReadInt();
	m_position.resize(num);
	m_normal.resize(normComp == 1 ? num : 0);
	m_color.resize(colComp == 1 ? num : 0);

	for (int i = 0; i < m_position.size(); i++)
	{
		m_position[i] = reader.ReadVec3();

		if (m_normal.size() > 0)
		{
			m_normal[i] = reader.ReadVec3();
		}

		if (m_color.size() > 0)
		{
			m_color[i] = reader.ReadVec3();
		}
	}

	reader.Close();
}
void PointCloud::OutputText(const std::string& name)
{
	KI::FileWriter writer;
	writer.Open(name);

	int num = m_position.size();
	for (int i = 0; i < m_position.size(); i++) {
		writer.Write(std::to_string(m_position[i].x) + " " + std::to_string(m_position[i].y) + " " + std::to_string(m_position[i].z));
		if (m_normal.size()) {
			writer.Write(" " + std::to_string(m_normal[i].x) + " " + std::to_string(m_normal[i].y) + " " + std::to_string(m_normal[i].z));
		}

		if (m_color.size()) {
			writer.Write(" " + std::to_string(m_color[i].x) + " " + std::to_string(m_color[i].y) + " " + std::to_string(m_color[i].z));
		}

		writer.Write("", true);
	}
}
// Format 
// Position Num, norm, color, etc.
// Position Num, 3, -1 etc. if not exist = 0 else other.
void PointCloud::OutputBinary(const std::string& name)
{
	KI::FileWriter writer;
	writer.Open(name, true);

	int num = m_position.size();
	writer.WriteBinary((void*)&num, KI::Format::INT);
	
	int norm = m_normal.size() != 0 ? 1 : 0;
	writer.WriteBinary((void*)&norm, KI::Format::INT);

	int col = m_color.size() != 0 ? 1 : 0;
	writer.WriteBinary((void*)&col, KI::Format::INT);

	for (int i = 0; i < m_position.size(); i++) {
		writer.WriteBinary((void*)&m_position[i], KI::Format::VEC3);
		if (m_normal.size() > 0) {
			writer.WriteBinary((void*)&m_normal[i], KI::Format::VEC3);
		}
		if (m_color.size() > 0) {
			writer.WriteBinary((void*)&m_color[i], KI::Format::VEC3);
		}
	}
	
	writer.Close();
}

