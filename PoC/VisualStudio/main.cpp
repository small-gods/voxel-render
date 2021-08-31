#include "graph.h"
#include "base.h"
#include "octotree.h"

#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <stdio.h>
#include <algorithm>


struct Material
{
	Color color;
	float reflection;
	float transparency;
	float refraction;
};


#include "graph.h"
#include "octotree.h"
#include <iostream>
#include <vector>




std::vector<Material> materialTable;


void sliceOctotree(const OctoTree<3>& tree, Canvas& canvas)
{
	int size = tree.size();
	for (int x = 0; x < size; ++x)
		for (int y = 0; y < size; ++y)
			for (int z = 0; z < size; ++z)
				canvas.setPixel(y, z + 150, materialTable[tree.getMaterial({ x, y, z })].color);
}




int main()
{
	Canvas canvas;

	OctoTree<3> tree(7);
	VoxelDriver<3> driver(tree);
	driver.FillRectangle({ 10, 10, 10 }, { 50, 50, 50 }, 1);
	driver.FillCircle({ 30, 70, 80 }, 30, 2);

	materialTable.push_back({ {1.0, 1.0, 1.0}, 0, 0, 0 });
	materialTable.push_back({ {0.5, 0.5, 0}, 0, 0, 0 });
	materialTable.push_back({ {0.0, 0.5, 0}, 0, 0, 0 });

	sliceOctotree(tree, canvas);
}
