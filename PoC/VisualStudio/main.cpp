#include "graph.h"
#include "base.h"
#include "octotree.h"

#include <iostream>
#include <array>
#include <string>
#include <vector>
#include <stdio.h>
#include <algorithm>
#include <chrono>


namespace chn = std::chrono;

double DeltaTime() {
	static chn::steady_clock::time_point old_time = chn::steady_clock::now();
	chn::steady_clock::time_point new_time = chn::steady_clock::now();
	double last_dt = (chn::duration_cast<chn::duration<double, std::ratio<1>>>(new_time - old_time)).count();
	old_time = new_time;
	return last_dt;
}


std::vector<Material> materialTable{
	{ {0.0, 0.0, 0.0}, 0, 0, 0 , false},
	{ {1.0, 1.0, 1.0}, 0, 0, 0 , false},
	{ {0.5, 0.5, 0}, 0, 0, 0, false },
	{ {0.0, 0, 0.5}, 0, 0, 0, false },
	{ {1.0, 1.0, 1.0}, 0, 0, 0, true }
};


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
	int size = 500;
	int deviations = 3;
	Canvas canvas(100, 150, size, size);

	Matrix<3, 128>* tree = new Matrix<3, 128>;
	VoxelDriver<Matrix<3, 128>, 3> driver(*tree);
	driver.FillRectangle({ 0, 0, 0 }, { 1, 128, 128 }, 1);
	driver.FillRectangle({ 0, 0, 0 }, { 128, 1, 128 }, 1);
	driver.FillRectangle({ 0, 0, 0 }, { 128, 128, 1 }, 4);
	driver.FillRectangle({ 127, 0, 0 }, { 1, 128, 128 }, 1);
	driver.FillRectangle({ 0, 127, 0 }, { 128, 1, 128 }, 1);
	driver.FillRectangle({ 0, 0, 127 }, { 128, 128, 1 }, 1);

	driver.FillRectangle({ 10, 10, 80 }, { 20, 20, 20 }, 3);
	driver.FillCircle({ 30, 70, 80 }, 30, 2);


	//sliceOctotree(*tree, canvas);

	DeltaTime();
	for (int y = 0; y < size; ++y)
		for (int x = 0; x < size; ++x)
		{
			Color color = {0, 0, 0};
			for (int i = 0; i < deviations; ++i)
				for (int j = 0; j < deviations; ++j)
					color = color + tree->Trace({{125.1, 64.1, 70.1},
								fVector<3>{-size / 2.0f,
									(float)(x - size / 2) + i / (float) deviations,
									(float)(y - size / 2) + j / (float) deviations
									}.Norm() });
			canvas.setPixel(x, y, color / (deviations * deviations));
		}
	double time = DeltaTime();
	std::cout << time << std::endl;

	canvas.Draw();
	delete tree;
}
