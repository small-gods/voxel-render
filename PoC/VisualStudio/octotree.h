#pragma once
#include "graph.h"
#include "base.h"
#include <Windows.h>
#include <stdexcept>
#include <stdio.h>
#include <vector>


struct Material
{
	Color color;
	float reflection;
	float transparency;
	float refraction;
	bool light;
};


extern std::vector<Material> materialTable;

template <size_t Dimension>
struct Ray
{
	fPoint<Dimension> point;
	fVector<Dimension> vector;
};


template <size_t Dimension>
class OctoTree
{
	using index_p = IndexPoint<Dimension>;

	class Node
	{
		NDimensionalMatrix<Node*, Dimension, 2> data;

		static constexpr bool isPointer(Node* ptr)
		{
			return !(((unsigned long long)ptr) & 1);
		}

		Node* getMonomaterial() const
		{
			return data[index_p(0)];
		}

		bool isMonomaterial() const
		{
			const Node* monomaterial = getMonomaterial();
			return index_p::all(2, [&](const index_p& i) { return data[i] == monomaterial; });
		}

		static constexpr int extractIndex(Node* ptr)
		{
			return (int)(((unsigned long long)ptr) >> 1);
		}

		static constexpr Node* injectIndex(int index)
		{
			return (Node*)((index << 1) | 1);
		}

		bool _setMaterial(index_p p, Node* material, int depth)
		{
			Node*& node = data[p >> depth];
			if (depth == 0)
			{
				node = material;
				return isMonomaterial();
			}

			if (!isPointer(node))
			{
				if (node == material)
					return false;
				node = new Node(node);
			}

			int mask = ~((-1) << depth);
			bool node_monomaterial = node->_setMaterial(p & mask, material, depth - 1);
			if (node_monomaterial)
			{
				Node* monomaterial = node->getMonomaterial();
				delete node;
				node = monomaterial;
			}
			return isMonomaterial();
		}

		int _getMaterial(index_p p, int depth) const
		{
			Node* node = data[p >> depth];
			if (!isPointer(node))
				return extractIndex(node);
			int mask = ~((-1) << depth);
			return node->_getMaterial(p & mask, depth - 1);
		}

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

		Node(Node* material) : data(material)
		{ }
	public:
		Node(): Node((Node*)1)
		{ }

		void setMaterial(index_p p, int index, int depth)
		{
			_setMaterial(p, injectIndex(index), depth);
		}

		int getMaterial(index_p p, int depth) const
		{
			return _getMaterial(p, depth);
		}

		~Node()
		{
			NDimensionalMatrix<Node*, Dimension, 2>& root = data;
			index_p::forEach(2, [&](const index_p& i) {
				if (isPointer(root[i]))
					delete root[i];
				});
		}
	};

	Node root;
	int depth;

public:
	OctoTree(int depth) : depth(depth - 1), root() {}

	void setMaterial(index_p p, int index)
	{
		root.setMaterial(p, index, depth);
	}

	int getMaterial(index_p p) const
	{
		return root.getMaterial(p, depth);
	}

	//Color Trace(Ray ray)
	//{
	//	throw std::bad_exception();
	//}

	int size() const
	{
		return 1 << (depth + 1);
	}
};

template <size_t Dimension, size_t Size>
class Matrix
{
	using index_p = IndexPoint<Dimension>;
	using f_point = fPoint<Dimension>;
	using f_vector = fVector<Dimension>;
	using i_vector = iVector<Dimension>;
	NDimensionalMatrix<int, Dimension, Size> data;

	struct TraceContext
	{
		Color color;
		int depth;
	};

	struct Intersetcion
	{
		int m;
		float t;
		int side;
	};

	template <int Dim>
	int MinIndex(int min, const f_point& p)
	{
		return MinIndex<Dim - 1>(p[Dim] < p[min] ? Dim : min, p);
	}

	template <>
	int MinIndex<-1>(int min, const f_point& p)
	{
		return min;
	}

	int MinIndex(const f_point& p)
	{
		return MinIndex<Dimension - 2>(Dimension - 1, p);
	}

	float random()
	{
		return (rand() * RAND_MAX + rand()) / (float) (RAND_MAX * RAND_MAX);
	}

	f_vector randomDirection()
	{
		f_vector v;
		do {
			for (int i = 0; i < Dimension; ++i)
				v[i] = random() - 0.5;
		} while (v.Len() > 0.5);
		return v.Norm();
	}

	Color ProcessingMaterial(
		const TraceContext& context,
		const Ray<Dimension>& ray,
		const Intersetcion& inter)
	{
		const Material& material = materialTable[inter.m];
		if (material.light)
			return context.color * material.color;
		if (context.depth > 3)
			return { 0, 0, 0 };
		f_vector rand_vec = randomDirection();
		rand_vec[inter.side] = std::abs(rand_vec[inter.side]) * (ray.vector[inter.side] > 0 ? -1 : 1);
		return Trace( {ray.point + ray.vector * (inter.t - 0.0001f), rand_vec },
			{ context.color * material.color, context.depth + 1 });
	}

	Color Trace(const Ray<Dimension>& ray, const TraceContext& context)
	{
		index_p pos, step;
		f_point next, len;
		for (int i = 0; i < Dimension; ++i)
		{
			pos[i] = (int)ray.point[i];
			step[i] = ray.vector[i] > 0 ? 1 : -1;
			len[i] = std::abs(1 / ray.vector[i]);
			next[i] = len[i] * (ray.vector[i] > 0 ? 1 - (ray.point[i] - pos[i]) : ray.point[i] - pos[i]);
		}

		int material = getMaterial(pos);
		while (true)
		{
			int min_index = MinIndex(next);
				
			pos[min_index] += step[min_index];
			if (pos[min_index] < 0 || pos[min_index] >= Size)
				return {0, 0, 0};

			if (int m = getMaterial(pos); material != m)
				return ProcessingMaterial(context, ray, { m, next[min_index], min_index});

			next[min_index] += len[min_index];
		}
	}
public:
	Matrix() : data(0)
	{ }

	void setMaterial(index_p p, int index)
	{
		data[p] = index;
	}

	int getMaterial(index_p p) const
	{
		return data[p];
	}

	Color Trace(Ray<Dimension> ray)
	{
		return Trace(ray, { {1, 1, 1}, 0 });
	}

	int size() const
	{
		return Size;
	}

};

template <typename T, size_t Dimension>
class VoxelDriver
{
	using index_p = IndexPoint<Dimension>;
	using vect_p = iVector<Dimension>;

	T& tree;
	int size;

public:
	VoxelDriver(T& tree) :
		tree(tree),
		size(tree.size())
	{ }

	void FillRectangle(const index_p& p, const vect_p& rect, int material)
	{
		index_p::forEach(index_p::Max(p, 0), index_p::Min(p + rect, size),
			[&](const index_p& i) {
				tree.setMaterial(i, material);
			});
	}

	void FillCircle(index_p p, int r, int material)
	{
		vect_p vect_r = vect_p(r);
		index_p::forEach(index_p::Max(p - vect_r, 0), index_p::Min(p + vect_r, size),
			[&](const index_p& i) {
				if ((p - i).Sqr() < r * r)
					tree.setMaterial(i, material);
			});
	}
};
