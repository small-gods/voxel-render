#pragma once
#include <initializer_list>
#include <math.h>

template <typename T, size_t Dimension>
class BasePoint
{
	T data[Dimension];

public:
	using const_iterator = const int*;
	using iterator = int*;

	BasePoint() {}

	BasePoint(std::initializer_list<T> input)
	{
		T* ptr = data;
		for (const T& v : input)
			*ptr++ = v;
	}

	BasePoint(const T& v)
	{
		for (size_t i = 0; i < Dimension; ++i)
			data[i] = v;
	}

	T& operator[](size_t i)
	{
		return data[i];
	}

	const T& operator[](size_t i) const
	{
		return data[i];
	}

	const_iterator cbegin() const
	{
		return (int*)data;
	}

	const_iterator cend() const
	{
		return ((int*)data) + Dimension;
	}

	iterator begin()
	{
		return (int*)data;
	}

	iterator end()
	{
		return ((int*)data) + Dimension;
	}
};


template <typename T>
class BasePoint<T, 0>
{
public:
	using const_iterator = const int*;

	BasePoint() {}

	BasePoint(std::initializer_list<T> input) {}

	const_iterator cbegin() const
	{
		return nullptr;
	}

	const_iterator cend() const
	{
		return nullptr;
	}
};


template <typename T, size_t Dimension>
class Vector : public BasePoint<T, Dimension>
{
public:
	Vector() : BasePoint<T, Dimension>() {}
	Vector(const T& v) : BasePoint<T, Dimension>(v) {}
	Vector(std::initializer_list<T> input) : BasePoint<T, Dimension>(input) {}

	Vector operator*(T t) const
	{
		Vector result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] * t;
		return result;
	}

	Vector operator/(T t) const
	{
		Vector result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] / t;
		return result;
	}

	Vector operator+(const Vector& v) const
	{
		Vector result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] + v[i];
		return result;
	}

	Vector operator-(const Vector& v) const
	{
		Vector result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] - v[i];
		return result;
	}

	T Dot(const Vector& v) const
	{
		T result = 0;
		for (int i = 0; i < Dimension; ++i)
			result += (*this)[i] * v[i];
		return result;
	}

	T Sqr()const
	{
		return this->Dot(*this);
	}

	T Len()const
	{
		return std::sqrt(Sqr());
	}

	Vector Norm()const
	{
		return *this / Len();
	}
};


template <typename T, size_t Dimension>
class Point : public BasePoint<T, Dimension>
{
	using Vect = Vector<T, Dimension>;

public:
	Point() : BasePoint<T, Dimension>() {}
	Point(const T& v) : BasePoint<T, Dimension>(v) {}
	Point(std::initializer_list<T> input) : BasePoint<T, Dimension>(input) {}

	Point operator+(const Vect& v) const
	{
		Point result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] + v[i];
		return result;
	}

	Vect operator-(const Point& p) const
	{
		Vect result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] - p[i];
		return result;
	}
};

template <size_t Dimension>
class IndexPoint : public Point<int, Dimension>
{
	template <typename Action>
	struct Algorithms
	{
		template <size_t Dim>
		static void forEach(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			constexpr size_t Next = Dim - 1;
			for (value[Next] = p1[Next]; value[Next] < p2[Next]; ++value[Next])
				forEach<Next>(p1, p2, action, value);
		}

		template <>
		static void forEach<0>(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			action(value);
		}

		template <size_t Dim>
		static bool exists(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			constexpr size_t Next = Dim - 1;
			for (value[Next] = p1[Next]; value[Next] < p2[Next]; ++value[Next])
				if (exists<Next>(p1, p2, action, value))
					return true;
			return false;
		}

		template <>
		static bool exists<0>(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			return action(value);
		}

		template <size_t Dim>
		static bool all(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			constexpr size_t Next = Dim - 1;
			for (value[Next] = p1[Next]; value[Next] < p2[Next]; ++value[Next])
				if (!all<Next>(p1, p2, action, value))
					return false;
			return true;
		}

		template <>
		static bool all<0>(const IndexPoint& p1, const IndexPoint& p2, Action action, IndexPoint& value)
		{
			return action(value);
		}
	};

	using Vect = Vector<int, Dimension>;

public:
	IndexPoint() : Point<int, Dimension>() {}
	IndexPoint(int v) : Point<int, Dimension>(v) {}
	IndexPoint(std::initializer_list<int> input) : Point<int, Dimension>(input) {}

	IndexPoint operator+(const Vect& v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] + v[i];
		return result;
	}

	IndexPoint operator-(const Vect& v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] - v[i];
		return result;
	}

	Vect operator-(const IndexPoint& p) const
	{
		Vect result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] - p[i];
		return result;
	}

	IndexPoint operator*(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] * v;
		return result;
	}

	IndexPoint operator/(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] / v;
		return result;
	}

	IndexPoint operator>>(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] >> v;
		return result;
	}

	IndexPoint operator<<(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] << v;
		return result;
	}

	IndexPoint operator&(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] & v;
		return result;
	}

	IndexPoint operator|(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] | v;
		return result;
	}

	IndexPoint operator^(int v) const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = (*this)[i] ^ v;
		return result;
	}

	IndexPoint operator~() const
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = ~(*this)[i];
		return result;
	}

	template <typename Action>
	static void forEach(const IndexPoint& p1, const IndexPoint& p2, Action action)
	{
		IndexPoint counter;
		Algorithms<Action>::template forEach<Dimension>(p1, p2, action, counter);
	}

	template <typename Action>
	static void forEach(const IndexPoint& p, Action action)
	{
		forEach(0, p, action);
	}

	template <typename Action>
	static bool exists(const IndexPoint& p1, const IndexPoint& p2, Action action)
	{
		IndexPoint counter;
		return Algorithms<Action>::template exists<Dimension>(p1, p2, action, counter);
	}

	template <typename Action>
	static bool exists(const IndexPoint& p, Action action)
	{
		return exists(0, p, action);
	}

	template <typename Action>
	static bool all(const IndexPoint& p1, const IndexPoint& p2, Action action)
	{
		IndexPoint counter;
		return Algorithms<Action>::template all<Dimension>(p1, p2, action, counter);
	}

	template <typename Action>
	static bool all(const IndexPoint& p, Action action)
	{
		return all(0, p, action);
	}

	static IndexPoint Min(const IndexPoint& p1, const IndexPoint& p2)
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = min(p1[i], p2[i]);
		return result;
	}

	static IndexPoint Max(const IndexPoint& p1, const IndexPoint& p2)
	{
		IndexPoint result;
		for (int i = 0; i < Dimension; ++i)
			result[i] = max(p1[i], p2[i]);
		return result;
	}
};


template <typename T, size_t Dim, size_t Size>
class NDimensionalMatrix
{
	using subtree = NDimensionalMatrix<T, Dim - 1, Size>;
	using index_p = IndexPoint<Dim>;
	subtree data[Size];

	T& get(typename index_p::const_iterator it) { return data[*it].get(it + 1); }
	const T& get(typename index_p::const_iterator it) const { return data[*it].get(it + 1); }

	friend class NDimensionalMatrix<T, Dim + 1, Size>;

public:
	NDimensionalMatrix() {}
	NDimensionalMatrix(const T& val)
	{
		for (int i = 0; i < Size; ++i)
			data[i] = subtree(val);
	}

	subtree& operator[](size_t i) { return data[i]; }
	const subtree& operator[](size_t i) const { return data[i]; }

	T& operator[](index_p p) { return get(p.cbegin()); }
	const T& operator[](index_p p) const { return get(p.cbegin()); }
};


template <typename T, size_t Size>
class NDimensionalMatrix<T, 0, Size>
{
	using index_p = IndexPoint<0>;
	T data;

	T& get(typename index_p::const_iterator it) { return data; }
	const T& get(typename index_p::const_iterator it) const { return data; }

	friend class NDimensionalMatrix<T, 1, Size>;

public:
	NDimensionalMatrix() {}
	NDimensionalMatrix(const T& val) : data(val) { }

	operator T& () { return data; }
	operator const T& () const { return data; }

	T& operator[](index_p p) { return data; }
	const T& operator[](index_p p) const { return data; }
};

template <typename T, size_t Depth>
class FixedStack
{
	T data[Depth];
	int frame;
public:
	FixedStack(): frame(-1)
	{}

	bool empty() const
	{
		return frame == -1;
	}

	int size()
	{
		return frame;
	}

	const T& top() const
	{
		return data[frame];
	}

	T& top()
	{
		return data[frame];
	}

	void push(T&& val) {
		++frame;
		data[frame] = val;
	}

	void push(const T& val) {
		++frame;
		data[frame] = val;
	}

	void pop()
	{
		--frame;
	}
};

template <size_t Dimension>
using iPoint = Point<int, Dimension>;

template <size_t Dimension>
using fPoint = Point<float, Dimension>;

template <size_t Dimension>
using dPoint = Point<double, Dimension>;


template <size_t Dimension>
using iVector = Vector<int, Dimension>;

template <size_t Dimension>
using fVector = Vector<float, Dimension>;

template <size_t Dimension>
using dVector = Vector<double, Dimension>;

