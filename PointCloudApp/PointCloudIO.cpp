#include "PointCloudIO.h"
#include "FileUtility.h"
#include "Random.h"
#include <iostream>
#include <string>
#include "PointCloud.h"

namespace KI
{
void PointCloudIO::LoadPCD(PointCloud* pPointCloud, const String& name)
{
	KI::FileReader reader;
	reader.Open(name);
	String contents;
	int line = -1;
	int index = 0;
	while (reader.ReadLine(contents)) {
		line++;
		if (line == 2) {
			auto split = KI::FileUtility::Split(contents, ' ');
		}
		if (line == 9) {
			auto split = KI::FileUtility::Split(contents, ' ');
			int size = std::stoi(split[1]);
			pPointCloud->m_position.resize(size);
			pPointCloud->m_normal.resize(size);
			pPointCloud->m_color.resize(size);
			pPointCloud->m_curvature.resize(size);
		}
		if (line < 11) { continue; }

		auto data = KI::FileUtility::Split(contents, ' ');

		pPointCloud->m_position[index].x = std::stof(data[0]);
		pPointCloud->m_position[index].y = std::stof(data[1]);
		pPointCloud->m_position[index].z = std::stof(data[2]);

		pPointCloud->m_normal[index].x = std::stof(data[0]);
		pPointCloud->m_normal[index].y = std::stof(data[1]);
		pPointCloud->m_normal[index].z = std::stof(data[2]);


		//pPointCloud->m_normal[index].x = std::stof(data[3]);
		//pPointCloud->m_normal[index].y = std::stof(data[4]);
		//pPointCloud->m_normal[index].z = std::stof(data[5]);

		//pPointCloud->m_curvature[index] = std::stof(data[6]);
		index++;
	}
}

void PointCloudIO::LoadXYZ(PointCloud* pPointCloud, const String& name)
{
	KI::FileReader reader;
	reader.Open(name);
	String contents;
	int line = -1;
	int index = 0;
	while (reader.ReadLine(contents)) {
		auto line = KI::FileUtility::Split(contents, ' ');
		assert(line.size() == 7);

		pPointCloud->m_position.push_back(Vector3(std::stof(line[0]), std::stof(line[1]), std::stof(line[2])));
		pPointCloud->m_color.push_back(Vector4(std::stof(line[4]) / 255.0, std::stof(line[5]) / 255.0, std::stof(line[6]) / 255.0, 1.0));
	}
}

PointCloud* PointCloudIO::Load(const String& name)
{
	auto pInstance = new PointCloud();
	auto ext = KI::FileUtility::GetExtension(name);
	pInstance->m_filePath = name;
	auto binPath = name + ".ki_bin";
	bool loadBin = false;
	if (ext == "ki_bin") {
		LoadBin(pInstance, name);
		loadBin = true;
	} else if (KI::FileUtility::IsExist(binPath)) {
		LoadBin(pInstance, binPath);
		loadBin = true;
	} else if (ext == "pcd") {
		LoadPCD(pInstance, name);
	} else {
		LoadXYZ(pInstance, name);
	}

	if (!loadBin) {
		OutputBinary(pInstance, binPath);
	}

	pInstance->m_index.resize(pInstance->m_position.size());
	for (int i = 0; i < pInstance->m_position.size(); i++) {
		pInstance->m_index[i] = i;
	}
	return pInstance;
}

PointCloud* PointCloudIO::Create2D(int positionNum, const glm::vec2& min, const glm::vec2& max)
{
	auto pInstance = new PointCloud();
	pInstance->m_position.resize(positionNum);
	pInstance->m_normal.resize(positionNum, Vector3(0));	// FileIOópÇ…çÏÇ¡ÇƒÇ¢ÇÈÅB
	for (int i = 0; i < positionNum; i++) {
		pInstance->m_position[i].x = Gaccho::rnd(min.x, max.x) / 100.0f;
		pInstance->m_position[i].y = Gaccho::rnd(min.y, max.y) / 100.0f;
		pInstance->m_position[i].z = 0.0f;
	}

	return pInstance;
}

void PointCloudIO::LoadBin(PointCloud* pPointCloud, const String& name)
{
	KI::FileReader reader;
	reader.Open(name, true);
	int num = reader.ReadInt();
	int normComp = reader.ReadInt();
	int colComp = reader.ReadInt();
	pPointCloud->m_position.resize(num);
	pPointCloud->m_normal.resize(normComp == 1 ? num : 0);
	pPointCloud->m_color.resize(colComp == 1 ? num : 0);

	for (int i = 0; i < pPointCloud->m_position.size(); i++) {
		pPointCloud->m_position[i] = reader.ReadVec3();

		if (pPointCloud->m_normal.size() > 0) {
			pPointCloud->m_normal[i] = reader.ReadVec3();
		}

		if (pPointCloud->m_color.size() > 0) {
			pPointCloud->m_color[i] = Vector4(reader.ReadVec3(),1.0);
		}
	}

	reader.Close();
}
void PointCloudIO::OutputText(PointCloud* pPointCloud, const String& name)
{
	KI::FileWriter writer;
	writer.Open(name);

	int num = pPointCloud->m_position.size();
	for (int i = 0; i < pPointCloud->m_position.size(); i++) {
		writer.Write(std::to_string(pPointCloud->m_position[i].x) + " " + std::to_string(pPointCloud->m_position[i].y) + " " + std::to_string(pPointCloud->m_position[i].z));
		if (pPointCloud->m_normal.size()) {
			writer.Write(" " + std::to_string(pPointCloud->m_normal[i].x) + " " + std::to_string(pPointCloud->m_normal[i].y) + " " + std::to_string(pPointCloud->m_normal[i].z));
		}

		if (pPointCloud->m_color.size()) {
			writer.Write(" " + std::to_string(pPointCloud->m_color[i].x) + " " + std::to_string(pPointCloud->m_color[i].y) + " " + std::to_string(pPointCloud->m_color[i].z));
		}

		writer.Write("", true);
	}
}
// Format 
// Position Num, norm, color, etc.
// Position Num, 3, -1 etc. if not exist = 0 else other.
void PointCloudIO::OutputBinary(PointCloud* pPointCloud, const String& name)
{
	KI::FileWriter writer;
	writer.Open(name, true);

	int num = pPointCloud->m_position.size();
	writer.WriteBinary((void*)&num, KI::Format::INT);

	int norm = pPointCloud->m_normal.size() != 0 ? 1 : 0;
	writer.WriteBinary((void*)&norm, KI::Format::INT);

	int col = pPointCloud->m_color.size() != 0 ? 1 : 0;
	writer.WriteBinary((void*)&col, KI::Format::INT);

	for (int i = 0; i < pPointCloud->m_position.size(); i++) {
		writer.WriteBinary((void*)&pPointCloud->m_position[i], KI::Format::VEC3);
		if (pPointCloud->m_normal.size() > 0) {
			writer.WriteBinary((void*)&pPointCloud->m_normal[i], KI::Format::VEC3);
		}
		if (pPointCloud->m_color.size() > 0) {
			writer.WriteBinary((void*)&pPointCloud->m_color[i], KI::Format::VEC3);
		}
	}

	writer.Close();
}

}