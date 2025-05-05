#ifndef KI_PARAMETER_H
#define KI_PARAMETER_H
namespace KI
{
class Parameter
{
public:
	Parameter() { m_pValue = nullptr; }
	Parameter(const String& name, const std::vector<float>& value);
	~Parameter() {};

	const String& Name() const { return m_name; }
	int Size() const { return (int)m_pValue->size(); }
	float Min() const { return m_min; }
	float Max() const { return m_max; }
	float Median() const { return m_median; }
	float Sum() const { return m_sum; }
	float Average() const { return m_average; }
	bool IsActive() const { return m_pValue != nullptr; }
	const std::vector<float>& Values() const { return *m_pValue; }
	const std::vector<float>& GetHistogram() const { return m_histogram.value; }
	std::vector<float> CreateHistogram(bool logScale, int count) const;
	Vector<Vector3> CreatePseudoColor() const;

private:
	void Calc();
	struct Histogram
	{
		Histogram() :logScale(false) {}
		bool logScale;
		std::vector<float> value;
	};

	String m_name;
	const std::vector<float>* m_pValue;
	float m_min;
	float m_max;
	float m_median;
	float m_sum;
	float m_average;
	Histogram m_histogram;
};


}
#endif KI_PARAMETER_H