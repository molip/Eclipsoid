#pragma once

#include "Technology.h"

#include <vector>
#include <set>

class TechTrack
{
public:
	TechTrack();

	void Add(TechType tech);
	bool Has(TechType tech) const;
	bool CanAdd(TechType techtype) const;

	int GetNextDiscount(Technology::Class c) const;
	static int GetDiscount(int tier);
	int GetCount(Technology::Class c) const;

private:
	std::vector<Technology> m_class[3];
	std::set<TechType> m_techs;
};
