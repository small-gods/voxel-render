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


template <typename T>
void sliceOctotree(const T& tree, Canvas& canvas)
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

	Matrix<3, 128>* tree = new Matrix<3, 128>;
	VoxelDriver<Matrix<3, 128>, 3> driver(*tree);
	driver.FillRectangle({ 10, 10, 10 }, { 50, 50, 50 }, 1);
	driver.FillRectangle({ 0, 0, 0 }, { 1, 128, 128 }, 1);
	driver.FillRectangle({ 0, 0, 0 }, { 128, 1, 128 }, 1);
	driver.FillRectangle({ 0, 0, 0 }, { 128, 128, 1 }, 1);
	driver.FillRectangle({ 127, 0, 0 }, { 1, 128, 128 }, 1);
	driver.FillRectangle({ 0, 127, 0 }, { 128, 1, 128 }, 1);
	driver.FillRectangle({ 0, 0, 127 }, { 128, 128, 1 }, 1);

	driver.FillCircle({ 30, 70, 80 }, 30, 2);

	materialTable.push_back({ {1.0, 1.0, 1.0}, 0, 0, 0 });
	materialTable.push_back({ {0.5, 0.5, 0}, 0, 0, 0 });
	materialTable.push_back({ {0.0, 0.5, 0}, 0, 0, 0 });

	//sliceOctotree(*tree, canvas);

	for (int x = 0; x < 500; ++x)
		for (int y = 0; y < 500; ++y)
		{
			float trace = tree->Trace({ {0, 0, 0}, {125, 64, 70}, {-250, (float)(x - 250), (float)(y - 250)} });
			//std::cout << trace << std::endl;
			canvas.setPixel(x + 100, y + 100, { 0, trace / 256, 0 });
		}
	delete tree;
}
