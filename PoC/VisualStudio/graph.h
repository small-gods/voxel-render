#pragma once

#include <Windows.h>
#undef min
#undef max

#include <cmath>
#include <algorithm>

struct Color
{
	float r;
	float g;
	float b;

	Color operator*(float c) const
	{
		return { r * c, g * c, b * c };
	}

	Color operator*(const Color& c) const
	{
		return { r * c.r, g * c.g, b * c.b };
	}

	Color operator/(float c) const
	{
		return { r / c, g / c, b / c };
	}

	Color operator+(Color c) const
	{
		return { r + c.r, g + c.g, b + c.b };
	}

	Color& operator+=(Color c)
	{
		return *this = *this + c;
	}

	Color operator-(Color c) const
	{
		return { r - c.r, g - c.g, b - c.b };
	}

	Color operator-() const
	{
		return { -r, -g, -b };
	}

	static int cut(float C, float k)
	{
		int v = (int)(k * C * 255);
		return std::min(255, v);
	}

	COLORREF toWinColor(float k = 1) const
	{
		return RGB(cut(r, k), cut(g, k), cut(b, k));
	}
};


class Canvas
{
	HDC hdc;
	int pos_x, pos_y;
	int width, height;
	
	COLORREF* matrix;

	int coord(int x, int y)
	{
		return x + y * height;
	}
public:
	Canvas(int x, int y, int width, int height):
		hdc(GetDC(GetConsoleWindow())),
		pos_x(x), pos_y(y),
		width(width), height(height),
		matrix(new COLORREF[width * height])
	{ }

	void setPixel(int x, int y, COLORREF color)
	{
		matrix[coord(x, y)] = color;
	}

	void setPixel(int x, int y, Color color, float k = 1)
	{
		matrix[coord(x, y)] = color.toWinColor(k);
	}

	void setPixel(float x, float y, Color color, float k = 1)
	{
		matrix[coord((int)x, (int)y)] = color.toWinColor(k);
	}

	void Draw()
	{
		for (int y = 0; y < height; ++y)
			for (int x = 0; x < width; ++x)
				SetPixel(hdc, x + pos_x, y + pos_y, matrix[coord(x, y)]);
	}

	~Canvas()
	{
		delete[] matrix;
	}
};
