#pragma once
#include "graph.h"
#include <Windows.h>
#include <stdexcept>


struct Ray
{
	Color color;
	Point point;
	Point vector;
};

class OctoTree
{
	class Node
	{
		Node* data[2][2][2];

		static constexpr bool isPointer(Node* ptr)
		{
			return !(((unsigned long long)ptr) & 1);
		}

		bool isMonomaterial() const
		{
			for (int x = 0; x < 2; ++x)
				for (int y = 0; y < 2; ++y)
					for (int z = 0; z < 2; ++z)
						if (data[x][y][z] != data[0][0][0])
							return false;
			return true;
		}

		Node* getMonomaterial() const
		{
			return data[0][0][0];
		}

		static constexpr int extractIndex(Node* ptr)
		{
			return (int)(((unsigned long long)ptr) >> 1);
		}

		static constexpr Node* injectIndex(int index)
		{
			return (Node*)((index << 1) | 1);
		}

		bool _setMaterial(int x, int y, int z, Node* material, int depth)
		{
#ifdef _DEBUG
			if ((x >> (depth + 1)) || (y >> (depth + 1)) || (z >> (depth + 1)))
				throw std::out_of_range("dimension out of range");
#endif // _DEBUG

			Node*& node = data[x >> depth][y >> depth][z >> depth];
			if (depth == 0)
			{
				node = material;
				return isMonomaterial();
			}

			if (!isPointer(node))
			{
				if (node == material)
					return false;
				node = new Node();
			}

			int mask = ~((-1) << depth);
			bool node_monomaterial = node->_setMaterial(x & mask, y & mask, z & mask, material, depth - 1);
			if (node_monomaterial)
			{
				Node* monomaterial = node->getMonomaterial();
				delete node;
				node = monomaterial;
			}
			return isMonomaterial();
		}

		int _getMaterial(int x, int y, int z, int depth) const
		{
#ifdef _DEBUG
			if ((x >> (depth + 1)) || (y >> (depth + 1)) || (z >> (depth + 1)))
				throw std::out_of_range("dimension out of range");
#endif // _DEBUG

			Node* node = data[x >> depth][y >> depth][z >> depth];
			if (!isPointer(node))
				return extractIndex(node);
			int mask = ~((-1) << depth);
			return node->_getMaterial(x & mask, y & mask, z & mask, depth - 1);
		}

		Node(const Node&) = delete;
		Node(Node&&) = delete;
		Node& operator=(const Node&) = delete;
		Node& operator=(Node&&) = delete;

	public:
		Node()
		{
			for (int x = 0; x < 2; ++x)
				for (int y = 0; y < 2; ++y)
					for (int z = 0; z < 2; ++z)
						data[x][y][z] = (Node*)1;
		}

		void setMaterial(int x, int y, int z, int index, int depth)
		{
			_setMaterial(x, y, z, injectIndex(index), depth);
		}

		int getMaterial(int x, int y, int z, int depth) const
		{
			return _getMaterial(x, y, z, depth);
		}

		~Node()
		{
			for (int x = 0; x < 2; ++x)
				for (int y = 0; y < 2; ++y)
					for (int z = 0; z < 2; ++z)
						if (isPointer(data[x][y][z]))
							delete data[x][y][z];
		}
	};

	Node root;
	int depth;

public:
	OctoTree(int depth) : depth(depth - 1), root() {}

	void setMaterial(int x, int y, int z, int index)
	{
		root.setMaterial(x, y, z, index, depth);
	}

	int getMaterial(int x, int y, int z) const
	{
		return root.getMaterial(x, y, z, depth);
	}

	Color Trace(Ray ray)
	{
		throw std::bad_exception();
	}

	int size() const
	{
		return 1 << (depth + 1);
	}
};


class VoxelDriver
{
	OctoTree& tree;
	int size;

	template<typename T>
	static constexpr T sqr(T x)
	{
		return x * x;
	}

public:
	VoxelDriver(OctoTree& tree) :
		tree(tree),
		size(tree.size())
	{ }

	void FillRectangle(int x, int y, int z, int width, int height, int depth, int material)
	{
		if (width < 0 || height < 0 || depth < 0)
			throw std::invalid_argument("negative dimension");

		for (int i = max(0, x); i < width && i < size; ++i)
			for (int j = max(0, y); j < height && j < size; ++j)
				for (int k = max(0, z); k < depth && k < size; ++k)
					tree.setMaterial(i, j, k, material);
	}

	void FillCircle(int x, int y, int z, int r, int material)
	{
		if (r < 0)
			throw std::invalid_argument("negative radius");

		for (int i = max(0, x - r); i < x + r && i < size; ++i)
			for (int j = max(0, y - r); j < y + r && j < size; ++j)
				for (int k = max(0, z - r); k < z + r && k < size; ++k)
					if (sqr(i - x) + sqr(j - y) + sqr(k - z) < sqr(r))
						tree.setMaterial(i, j, k, material);
	}
};
