#pragma once
#include "graph.h"
#include "base.h"
#include <Windows.h>
#include <stdexcept>



template <size_t Dimension>
struct Ray
{
	Color color;
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


template <size_t Dimension>
class VoxelDriver
{
	using octotree = OctoTree<Dimension>;
	using index_p = IndexPoint<Dimension>;
	using vect_p = iVector<Dimension>;

	octotree& tree;
	int size;

public:
	VoxelDriver(octotree& tree) :
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
