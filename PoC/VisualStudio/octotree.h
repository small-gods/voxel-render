#pragma once
#include "graph.h"
#include "base.h"
#include "random.h"
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


template <size_t Dimension, size_t Depth>
class OctoTree
{
	using index_p = IndexPoint<Dimension>;
	using f_point = fPoint<Dimension>;
	using f_vector = fVector<Dimension>;
	using i_vector = iVector<Dimension>;

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

	struct Node
	{
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

		NDimensionalMatrix<Node*, Dimension, 2> data;

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

	struct TraceContext
	{
		Color color;
		int depth;
	};

	Color ProcessingMaterial(
		const TraceContext& ctx,
		const Ray<Dimension>& ray,
		const Intersetcion& inter)
	{
		const Material& material = materialTable[inter.m];
		if (material.light)
			return ctx.color * material.color;

		if (ctx.depth > 3)
			return { 0, 0, 0 };

		f_point start_point = ray.point + ray.vector * (inter.t - 0.0001f);

		f_vector rand_vec = rnd.direction();
		rand_vec[inter.side] = std::abs(rand_vec[inter.side]) * (ray.vector[inter.side] > 0 ? -1 : 1);
		Color result = Trace(
			{ ctx.color * material.color, ctx.depth + 1 },
			{ start_point , rand_vec });

		if (material.reflection > 0)
		{
			f_vector reflect_vector = ray.vector;
			reflect_vector[inter.side] = -reflect_vector[inter.side];
			result = result + Trace(
				{ ctx.color * material.reflection, ctx.depth + 1 },
				{ start_point, reflect_vector });
		}
		return result;
	}

	Color Trace(const TraceContext& ctx, const Ray<Dimension>& ray)
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
			if (pos[min_index] < 0 || pos[min_index] >= size())
				return { 0, 0, 0 };

			if (int m = getMaterial(pos); material != m)
				return ProcessingMaterial(ctx, ray, { m, next[min_index], min_index });

			next[min_index] += len[min_index] ;
		}
	}

	Node root;
	Random<Dimension> rnd;

public:
	OctoTree(): root() {}

	void setMaterial(index_p p, int index)
	{
		root.setMaterial(p, index, Depth - 1);
	}

	int getMaterial(index_p p) const
	{
		return root.getMaterial(p, Depth - 1);
	}

	Color Trace(Ray<Dimension> ray)
	{
		TraceContext ctx{ {1, 1, 1}, 0 };
		return Trace(ctx, ray);
	}

	int size() const
	{
		return 1 << Depth;
	}
};

template <size_t Dimension, size_t Depth>
class Matrix
{
	using index_p = IndexPoint<Dimension>;
	using f_point = fPoint<Dimension>;
	using f_vector = fVector<Dimension>;
	using i_vector = iVector<Dimension>;
	using distanceMatrix = NDimensionalMatrix<byte, Dimension, 1 << Depth>;

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

	Color ProcessingMaterial(
		const TraceContext& context,
		const Ray<Dimension>& ray,
		const Intersetcion& inter)
	{
		if (inter.m < 0 || inter.m >= materialTable.size())
			printf("Xmmm...");
		const Material& material = materialTable[inter.m];
		if (material.light)
			return context.color * material.color;
		
		if (context.depth > 3)
			return { 0, 0, 0 };

		f_point start_point = ray.point + ray.vector * (inter.t - 0.0001f);

		f_vector rand_vec = rnd.direction();
		rand_vec[inter.side] = std::abs(rand_vec[inter.side]) * (ray.vector[inter.side] > 0 ? -1 : 1);
		Color result = Trace( { start_point , rand_vec },
			{ context.color * material.color, context.depth + 1 });
		
		if (material.reflection > 0)
		{
			f_vector reflect_vector = ray.vector;
			reflect_vector[inter.side] = -reflect_vector[inter.side];
			result = result + Trace({ start_point, reflect_vector },
				{ context.color * material.reflection, context.depth + 1 });
		}
		return result;
	}

	Color Trace(const Ray<Dimension>& ray, const TraceContext& context)
	{
		index_p pos, step;
		f_point next, len;
		Point<distanceMatrix*, Dimension> distances;
		byte minDist = 255;
		for (int i = 0; i < Dimension; ++i)
			pos[i] = (int)ray.point[i];
		
		for (int i = 0; i < Dimension; ++i)
		{
			distances[i] = ray.vector[i] > 0 ? &(backwardDistances[i]) : &(forwardDistances[i]);
			minDist = std::min((*(distances[i]))[pos], minDist);
		}
		minDist = std::max(minDist - 1, 0);
		Ray<Dimension> ray2 = ray;
		ray2.point = ray.point + ray.vector * (minDist / ray.vector.ManhattanDistance());

		for (int i = 0; i < Dimension; ++i)
		{
			pos[i] = (int)ray2.point[i];
			step[i] = ray2.vector[i] > 0 ? 1 : -1;
			len[i] = std::abs(1 / ray2.vector[i]);
			next[i] = len[i] * (ray2.vector[i] > 0 ? 1 - (ray2.point[i] - pos[i]) : ray2.point[i] - pos[i]);
		}

		int material = getMaterial(pos);
		while (true)
		{
			int min_index = MinIndex(next);

			pos[min_index] += step[min_index];
			if (pos[min_index] < 0 || pos[min_index] >= size())
				return {0, 0, 0};

			if (int m = getMaterial(pos); material != m)
				return ProcessingMaterial(context, ray2, { m, next[min_index], min_index});

			next[min_index] += len[min_index];
		}
	}


	template <size_t Line>
	void fillDimensionDistances(
		const NDimensionalMatrix<byte, Line, 1 << Depth>& lastDistances,
		NDimensionalMatrix<byte, Line, 1 << Depth>& distances,
		const NDimensionalMatrix<int, Line, 1 << Depth>& lastMaterials,
		const NDimensionalMatrix<int, Line, 1 << Depth>& materials,
		int direction, int layer, int lastLayer)
	{
		if (Dimension - Line == direction)
		{
			fillDimensionDistances<Line - 1>(
				lastDistances[lastLayer], distances[layer],
				lastMaterials[lastLayer], materials[layer],
				direction, layer, lastLayer);
			return;
		}
		for (int i = 0; i < size(); ++i)
		{
			fillDimensionDistances<Line - 1>(
				lastDistances[i], distances[i],
				lastMaterials[i], materials[i],
				direction, layer, lastLayer);
		}
	}

	template <>
	void fillDimensionDistances<0>(
		const NDimensionalMatrix<byte, 0, 1 << Depth>& lastDistances,
		NDimensionalMatrix<byte, 0, 1 << Depth>& distances,
		const NDimensionalMatrix<int, 0, 1 << Depth>& lastMaterials,
		const NDimensionalMatrix<int, 0, 1 << Depth>& materials,
		int direction, int layer, int lastLayer)
	{
		distances = lastMaterials == materials ? lastDistances + 1 : 0;
	}


	template <size_t Line>
	void fillLayer(
		NDimensionalMatrix<byte, Line, 1 << Depth>& distances,
		int direction, int layer, byte value)
	{
		if (Dimension - Line == direction)
		{
			fillLayer<Line - 1>(distances[layer], direction, layer, value);
			return;
		}
		for (int i = 0; i < size(); ++i)
			fillLayer<Line - 1>(distances[i], direction, layer, layer);
	}

	template <>
	void fillLayer<0>(
		NDimensionalMatrix<byte, 0, 1 << Depth>& distances,
		int direction, int layer, byte value)
	{
		distances = value;
	}


	template <size_t Line>
	bool dropLayer(
		NDimensionalMatrix<byte, Dimension, 1 << Depth>& distances,
		int direction, int layer, index_p& point)
	{
		if (Dimension - Line == direction)
		{
			return dropLayer<Line - 1>(distances, direction, layer, point);
		}
		bool f = true;
		for (int i = 0; i < size(); ++i)
		{
			point[Dimension - Line] = i;
			f &= dropLayer<Line - 1>(distances, direction, layer, point);
		}
		return f;
	}

	template <>
	bool dropLayer<0>(
		NDimensionalMatrix<byte, Dimension, 1 << Depth>& distances,
		int direction, int layer, index_p& point)
	{
		index_p p1, p2;
		for (int i = 0; i < Dimension; ++i)
		{
			if (i == direction)
			{
				p2[i] = (p1[i] = point[i]) + 1;
			}
			else
			{
				p1[i] = std::max(point[i] - 1, 0);
				p2[i] = std::min(point[i] + 1, size() - 1) + 1;
			}
		}
		byte min = 255;
		index_p::forEach(p1, p2, [&](const index_p& p) {
				byte v = distances[p];
				if (v < min) min = v;
			});
		byte& depth = distances[point];
		if (depth <= min + 1)
			return true;
		depth = min + 1;
		return false;
	}

	void dropLayer(NDimensionalMatrix<byte, Dimension, 1 << Depth>& distances, int direction, int layer)
	{
		index_p point;
		point[direction] = layer;
		while (!dropLayer<Dimension>(distances, direction, layer, point));
	}

	Random<Dimension> rnd;
	NDimensionalMatrix<int, Dimension, 1 << Depth> data;
	Point<distanceMatrix, Dimension> forwardDistances;
	Point<distanceMatrix, Dimension> backwardDistances;

public:
	Matrix() :
		data(0),
		forwardDistances(),
		backwardDistances()
	{ }

	void setMaterial(index_p p, int index)
	{
		data[p] = index;
	}

	int getMaterial(index_p p) const
	{
		return data[p];
	}

	void RecalculateDistances()
	{
		for (int direction = 0; direction < Dimension; ++direction)
		{
			fillLayer(forwardDistances[direction], direction, 0, 0);
			for (int i = 1; i < size(); ++i)
			{
				fillDimensionDistances(
					forwardDistances[direction],
					forwardDistances[direction],
					data, data, direction, i, i - 1);
				dropLayer(forwardDistances[direction], direction, i);
			}
			fillLayer(backwardDistances[direction], direction, size() - 1, 0);
			for (int i = size() - 2; i >= 0; --i)
			{
				fillDimensionDistances(
					backwardDistances[direction],
					backwardDistances[direction],
					data, data, direction, i, i + 1);
				dropLayer(backwardDistances[direction], direction, i);
			}
		}
	}
	Color Trace(Ray<Dimension> ray)
	{
		return Trace(ray, { {1, 1, 1}, 0 });
	}

	constexpr int size() const
	{
		return 1 << Depth;
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
