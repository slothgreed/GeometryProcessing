#include "Parameter.h"
#include "Utility.h"
namespace KI
{
Parameter::Parameter(const String& name, const std::vector<float>& value)
{
	m_name = name;
	m_pValue = &value;
	Calc();
}

void Parameter::Calc()
{
	m_min = (*m_pValue)[0];
	m_max = (*m_pValue)[0];
	float sum = 0;
	for (int i = 0; i < m_pValue->size(); i++) {
		auto value = (*m_pValue)[i];
		m_min = std::min(m_min, value);
		m_max = std::max(m_max, value);
		sum += value;
	}
	m_sum = sum;
	m_average = sum / m_pValue->size();

}

std::vector<float> Parameter::CreateHistogram(bool logScale, int binCount) const
{
	std::vector<float> histogram(binCount, 0.0f);
	for (float a : Values()) {
		float value = logScale ? std::log(glm::max(a, 1e-6f)) : a;
		float minVal = logScale ? std::log(glm::max(m_min, 1e-6f)) : m_min;
		float maxVal = logScale ? std::log(glm::max(m_max, 1e-6f)) : m_max;
		int bin = static_cast<int>(((value - minVal) / (maxVal - minVal)) * binCount);
		bin = std::clamp(bin, 0, binCount - 1);
		histogram[bin] += 1.0f;
	}

	return histogram;
}

Vector<Vector3> Parameter::CreatePseudoColor() const
{
	Vector<Vector3> vertexColor(m_pValue->size());
	for (int i = 0; i < vertexColor.size(); i++) {
		vertexColor[i] = ColorUtility::CreatePseudo(m_pValue->at(i), m_min, m_max);
	}
	return vertexColor;
}

}