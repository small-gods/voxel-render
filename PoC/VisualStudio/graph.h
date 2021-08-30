#pragma once

#include <Windows.h>
#include <cmath>

struct Point
{
	float x;
	float y;
	float z;

	Point operator*(float c) const
	{
		return { x * c, y * c, z * c };
	}

	Point operator*(const Point& p) const
	{
		return { x * p.x, y * p.y, z * p.z };
	}

	Point operator/(float c) const
	{
		return *this * (1 / c);
	}

	Point operator+(const Point& p) const
	{
		return { x + p.x, y + p.y, z + p.z };
	}

	Point operator-(const Point& p) const
	{
		return { x - p.x, y - p.y, z - p.z };
	}

	Point operator-() const
	{
		return { -x, -y, -z };
	}

	float Len()const
	{
		return std::sqrt(x * x + y * y + z * z);
	}

	Point Norm() const
	{
		return *this / Len();
	}

	float Dot(const Point& p)const
	{
		return x * p.x + y * p.y + z * p.z;
	}

	float Sqr()const
	{
		return this->Dot(*this);
	}

	Point Abs()const
	{
		return { std::abs(x), std::abs(y), std::abs(z) };
	}

	Point Min(const Point& other)
	{
		return Point{
			std::fminf(x, other.x),
			std::fminf(y, other.y),
			std::fminf(z, other.z)
		};
	}

	Point Max(const Point& other)
	{
		return Point{
			std::fmaxf(x, other.x),
			std::fmaxf(y, other.y),
			std::fmaxf(z, other.z)
		};
	}

	Point Cross(const Point& p)
	{
		return {
			y * p.z - z * p.y,
			z * p.x - x * p.z,
			x * p.y - y * p.x
		};
	}
};

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
