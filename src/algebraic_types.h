#pragma once
#include <array>
#include <cmath>
#include <type_traits>
#include <xmmintrin.h>

template <typename type, size_t dim>
class RealVector
{
public:
	std::array<type, dim> data;

public:
	RealVector<type, dim> inline operator+(const RealVector<type, dim> &other) const;
	RealVector<type, dim> inline operator-(const RealVector<type, dim> &other) const;
	RealVector<type, dim> inline operator*(type scalar) const;
	RealVector<type, dim> inline operator/(type scalar) const;

	void inline operator/=(type v);
	void inline operator*=(type scalar);
	void inline operator+=(const RealVector<type, dim> &other);
	void inline operator-=(const RealVector<type, dim> &other);

	bool operator==(const RealVector<type, dim> &other) const;

	type operator*(const RealVector<type, dim> &other) const;

	RealVector<type, 3> cross_product(const RealVector<type, 3> &other) const;

	type inline length() const
	{
		return std::sqrt(this->operator*(*this));
	}
};

template <typename type, size_t dim>
inline RealVector<type, dim> RealVector<type, dim>::operator+(const RealVector<type, dim> &other) const
{
	RealVector<type, dim> res;
	for (size_t i = 0; i < dim; i++)
	{
		res.data[i] = data[i] + other.data[i];
	}
	return res;
}

template <typename type, size_t dim>
inline RealVector<type, dim> RealVector<type, dim>::operator-(const RealVector<type, dim> &other) const
{
	RealVector<type, dim> res;
	for (size_t i = 0; i < dim; i++)
	{
		res.data[i] = data[i] - other.data[i];
	}
	return res;
}

template <typename type, size_t dim>
inline RealVector<type, dim> RealVector<type, dim>::operator*(type scalar) const
{
	RealVector<type, dim> res;
	for (size_t i = 0; i < dim; i++)
	{
		res.data[i] = data[i] * scalar;
	}
	return res;
}

template <typename type, size_t dim>
inline RealVector<type, dim> RealVector<type, dim>::operator/(type scalar) const
{
	RealVector<type, dim> res;
	for (size_t i = 0; i < dim; i++)
	{
		res.data[i] = data[i] / scalar;
	}
	return res;
}

template <typename type, size_t dim>
inline void RealVector<type, dim>::operator/=(type v)
{
	for (size_t i = 0; i < dim; i++)
	{
		data[i] /= v;
	}
}

template <typename type, size_t dim>
inline void RealVector<type, dim>::operator*=(type scalar)
{
	for (size_t i = 0; i < dim; i++)
	{
		data[i] *= scalar;
	}
}

template <typename type, size_t dim>
inline void RealVector<type, dim>::operator+=(const RealVector<type, dim> &other)
{
	for (size_t i = 0; i < dim; i++)
	{
		data[i] += other.data[i];
	}
}

template <typename type, size_t dim>
inline void RealVector<type, dim>::operator-=(const RealVector<type, dim> &other)
{
	for (size_t i = 0; i < dim; i++)
	{
		data[i] -= other.data[i];
	}
}

template <typename type, size_t dim>
inline bool RealVector<type, dim>::operator==(const RealVector<type, dim> &other) const
{
	return data == other.data;
}

template <typename type, size_t dim>
inline type RealVector<type, dim>::operator*(const RealVector<type, dim> &other) const
{
	type res = type(0);
	for (size_t i = 0; i < dim; i++)
	{
		res += data[i] * other.data[i];
	}
	return res;
}

template <typename type, size_t dim>
inline RealVector<type, 3> RealVector<type, dim>::cross_product(const RealVector<type, 3> &other) const
{
	static_assert(dim == 3);
	RealVector<type, 3> res;
	res.data[0] = data[1] * other.data[2] - data[2] * other.data[1];
	res.data[1] = data[2] * other.data[0] - data[0] * other.data[2];
	res.data[2] = data[0] * other.data[1] - data[1] * other.data[0];

	return res;
}

typedef RealVector<float, 3> float3;
typedef RealVector<unsigned int, 3> uint3;
typedef RealVector<int, 3> int3;