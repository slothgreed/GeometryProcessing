#ifndef SIMD_H
#define SIMD_H	

namespace KI
{
namespace SIMD
{
struct F32X8
{
	F32X8(__m256 value) : m_value(value) {}
	F32X8(float value) : m_value(_mm256_set1_ps(value)) {}
	F32X8(const float* value) : m_value(_mm256_loadu_ps(value)) {}
	
	void Store(float* value) const
	{
		_mm256_storeu_ps(value, m_value);
	}

	int GetMask() const
	{
		return _mm256_movemask_ps(m_value);
	}


	F32X8 operator+(const F32X8& other) const
	{
		return F32X8(_mm256_add_ps(m_value, other.m_value));
	}

	F32X8 operator-(const F32X8& other) const
	{
		return F32X8(_mm256_sub_ps(m_value, other.m_value));
	}

	F32X8 operator*(const F32X8& other) const
	{
		return F32X8(_mm256_mul_ps(m_value, other.m_value));
	}

	F32X8 operator/(const F32X8& other) const
	{
		return F32X8(_mm256_div_ps(m_value, other.m_value));
	}

	F32X8 operator>(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_GT_OS));
	}

	F32X8 operator>=(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_GE_OS));
	}

	F32X8 operator<(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_LT_OS));
	}

	F32X8 operator<=(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_LE_OS));
	}

	F32X8 operator==(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_EQ_OS));
	}

	F32X8 operator!=(const F32X8& other) const
	{
		return F32X8(_mm256_cmp_ps(m_value, other.m_value, _CMP_NEQ_OS));
	}

	F32X8 operator&(const F32X8& other) const
	{
		return F32X8(_mm256_and_ps(m_value, other.m_value));
	}

	static F32X8 Add(const F32X8& a, const F32X8& b)
	{
		return F32X8(_mm256_add_ps(a.m_value, b.m_value));
	}

	static F32X8 Sub(const F32X8& a, const F32X8& b)
	{
		return F32X8(_mm256_sub_ps(a.m_value, b.m_value));
	}

	static F32X8 Mul(const F32X8& a, const F32X8& b)
	{
		return F32X8(_mm256_mul_ps(a.m_value, b.m_value));
	}
	static F32X8 Div(const F32X8& a, const F32X8& b)
	{
		return F32X8(_mm256_div_ps(a.m_value, b.m_value));
	}
private:
	__m256 m_value;
};
}
}

#endif
