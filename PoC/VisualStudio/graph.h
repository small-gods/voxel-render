#pragma once

#include <Windows.h>
#include <cmath>

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
		return min(255, v);
	}

	COLORREF toWinColor(float k = 1) const
	{
		return RGB(cut(r, k), cut(g, k), cut(b, k));
	}
};


class Canvas
{
	HDC hdc;
public:
	Canvas(): hdc(GetDC(GetConsoleWindow())) { }

	void setPixel(int x, int y, COLORREF color)
	{
		SetPixel(hdc, x, y, color);
	}

	void setPixel(int x, int y, Color color, float k = 1)
	{
		setPixel(x, y, color.toWinColor(k));
	}

	void setPixel(float x, float y, Color color, float k = 1)
	{
		setPixel((int)x, (int)y, color.toWinColor(k));
	}
};
