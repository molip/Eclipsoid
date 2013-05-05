#pragma once

#include <vector>
#include "EnumRange.h"

enum class Resource { None = -1, Money, Science, Materials, _Count };

class Resources : private std::vector<int>
{
public:
	const int& operator[] (Resource r) const { return __super::at((int)r); }
	int& operator[] (Resource r) { return __super::at((int)r); }

	int GetTotal() const { int n = 0; for (auto& r : *this) n += r; return n; }
	bool IsEmpty() const { for (auto& r : *this) if (r) return false; return true; }

protected:
	Resources(int money = 0, int sci = 0, int mat = 0) 
	{
		// Same order as enum.
		push_back(money);
		push_back(sci);
		push_back(mat);
	}

private:
};

class Population : public Resources
{
public:
	Population() {}
	Population(int money, int sci, int mat) : Resources(money, sci, mat) {}
};

class Storage : public Resources
{
public:
	Storage() {}
	Storage(int money, int sci, int mat) : Resources(money, sci, mat) {}
};
