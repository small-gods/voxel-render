#pragma once
#include <random>

template <size_t Dimension>
class Random
{
	using f_vector = fVector<Dimension>;

public:
	float next()
	{
		return (rand() * RAND_MAX + rand()) / (float)(RAND_MAX * RAND_MAX);
	}

	f_vector direction()
	{
		f_vector v;
		do {
			for (int i = 0; i < Dimension; ++i)
				v[i] = next() - 0.5;
		} while (v.Len() > 0.5);
		return v.Norm();
	}
};
