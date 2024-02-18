#include "GLTFStruct.h"
#include "GLTFLoader.h"
#include <GLTFSDK/GLTF.h>
#include <GLTFSDK/GLTFResourceReader.h>
#include <GLTFSDK/GLBResourceReader.h>
#include <GLTFSDK/Deserialize.h>
#include "GLTFShader.h"
#include "GLBuffer.h"
namespace KI
{
const Microsoft::glTF::Mesh* GLTFNode::GetMesh(const GLTFDocument* pDocument)
{
	String id = pDocument->GetDocument().nodes.Get(m_index).meshId;
	if (id == "")
	{
		return nullptr;
	}
	else
	{
		return &pDocument->GetDocument().meshes.Get(StringToInt(id));
	}
}

Matrix4x4 CreateMatrix(const Microsoft::glTF::Matrix4& matrix)
{
	return Matrix4x4(
		matrix.values[0], matrix.values[1], matrix.values[2], matrix.values[3],
		matrix.values[4], matrix.values[5], matrix.values[6], matrix.values[7],
		matrix.values[8], matrix.values[9], matrix.values[10], matrix.values[11],
		matrix.values[12], matrix.values[13], matrix.values[14], matrix.values[15]);
}

Matrix4x4 CreateTranslate(Microsoft::glTF::Vector3 value)
{
	return Matrix4x4(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		value.x, value.y, value.z, 1);
}
const Matrix4x4& GLTFNode::GetMatrix(const GLTFDocument* pDocument)
{
	if (m_matrix.has_value())
	{
		return m_matrix.value();
	}
	
	auto node = pDocument->GetDocument().nodes.Get(m_index);
	m_matrix = CreateMatrix(node.matrix)* CreateTranslate(node.translation);

	return m_matrix.value();
}

const Microsoft::glTF::Accessor* GLTFNode::GetAccessor(const GLTFDocument* pDocument, const Microsoft::glTF::MeshPrimitive& primitive)
{
	return &pDocument->GetDocument().accessors.Get(primitive.indicesAccessorId);
}

void GLTFScene::Draw(const Matrix4x4& proj, const Matrix4x4& view)
{
	auto pShader = std::make_unique<GLTFShader>();
	pShader->Build();
	pShader->Use();
	pShader->SetViewProj(proj * view);
	pShader->SetModel(Matrix4x4(1.0f));
	for (const auto& mesh : m_meshes) {
		pShader->SetVertexBuffer(mesh.GetVertexBuffer().get(), mesh.GetFormats());
		pShader->SetIndexBuffer(mesh.GetIndexBuffer().get());
		for (const auto& primitive : mesh.GetPrimitives()) {
			const auto& material = m_material[primitive.materialIndex];
			pShader->BindMaterial(material);
			pShader->DrawElement(primitive, mesh.GetIndexBuffer()->DataType());
		}
	}
}

}