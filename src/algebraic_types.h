#pragma once
#include <array>
#include <cmath>
#include <type_traits>

template <typename type, unsigned int dim>
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

template <typename type, unsigned int dim>
inline RealVector<type, dim> RealVector<type, dim>::operator+(const RealVector<type, dim> &other) const
{
	RealVector<type, dim> res;
	for (int i = 0; i < dim; i++)
	{
		res.data[i] = data[i] + other.data[i];
	}
	return res;
}

template <typename type, unsigned int dim>
inline RealVector<type, dim> RealVector<type, dim>::operator-(const RealVector<type, dim> &other) const
{
	RealVector<type, dim> res;
	for (int i = 0; i < dim; i++)
	{
		res.data[i] = data[i] - other.data[i];
	}
	return res;
}

template <typename type, unsigned int dim>
inline RealVector<type, dim> RealVector<type, dim>::operator*(type scalar) const
{
	RealVector ret;
	for (int i = 0; i < dim; i++)
	{
		ret.data[i] = data[i] * scalar;
	}
	return ret;
}

template <typename type, unsigned int dim>
inline RealVector<type, dim> RealVector<type, dim>::operator/(type scalar) const
{
	RealVector ret;
	for (int i = 0; i < dim; i++)
	{
		ret.data[i] = data[i] / scalar;
	}
	return ret;
}

template <typename type, unsigned int dim>
inline void RealVector<type, dim>::operator/=(type v)
{
	for (int i = 0; i < dim; i++)
	{
		data[i] /= v;
	}
}

template <typename type, unsigned int dim>
inline void RealVector<type, dim>::operator*=(type scalar)
{
	for (int i = 0; i < dim; i++)
	{
		data[i] *= scalar;
	}
}

template <typename type, unsigned int dim>
inline void RealVector<type, dim>::operator+=(const RealVector<type, dim> &other)
{
	for (int i = 0; i < dim; i++)
	{
		data[i] += other.data[i];
	}
}

template <typename type, unsigned int dim>
inline void RealVector<type, dim>::operator-=(const RealVector<type, dim> &other)
{
	for (int i = 0; i < dim; i++)
	{
		data[i] -= other.data[i];
	}
}

template <typename type, unsigned int dim>
inline bool RealVector<type, dim>::operator==(const RealVector<type, dim> &other) const
{
	return data == other.data;
}

template <typename type, unsigned int dim>
inline type RealVector<type, dim>::operator*(const RealVector<type, dim> &other) const
{
	type ret = type(0);
	for (int i = 0; i < dim; i++)
	{
		ret += data[i] * other.data[i];
	}
	return ret;
}

template <typename type, unsigned int dim>
inline RealVector<type, 3> RealVector<type, dim>::cross_product(const RealVector<type, 3> &other) const
{
	static_assert(dim == 3);
	RealVector<type, 3> ret;
	ret.data[0] = data[1] * other.data[2] - data[2] * other.data[1];
	ret.data[1] = data[2] * other.data[0] - data[0] * other.data[2];
	ret.data[2] = data[0] * other.data[1] - data[1] * other.data[0];

	return ret;
}

typedef RealVector<float, 3> float3;
typedef RealVector<unsigned int, 3> uint3;
typedef RealVector<int, 3> int3;