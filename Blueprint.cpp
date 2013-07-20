#include "stdafx.h"
#include "Blueprint.h"
#include "BlueprintDefs.h"
#include "ShipLayout.h"
#include "Ship.h"
#include "Serial.h"
#include "IndexRange.h"

DEFINE_INDEXRANGE(SlotRange, Blueprint, ShipPart, GetSlot, GetSlotCount)

Blueprint::Blueprint() : m_pDef(nullptr)
{
}

Blueprint::Blueprint(RaceType r, ShipType s) : m_pDef(nullptr)
{
	Init(r, s);
	m_overlay.SetType(s);
}

Blueprint::Blueprint(const BlueprintDef& def) : m_pDef(&def), m_overlay(m_pDef->GetBaseLayout().GetType())
{
}

void Blueprint::Init(RaceType r, ShipType s) 
{
	m_pDef = &BlueprintDefs::Get(r, s);
}

ShipType Blueprint::GetType() const { return m_pDef->GetBaseLayout().GetType(); }
const ShipLayout& Blueprint::GetBaseLayout() const { return m_pDef->GetBaseLayout(); }
	
int Blueprint::GetFixedInitiative() const { return m_pDef->GetFixedInitiative(); }
int Blueprint::GetFixedPower() const { return m_pDef->GetFixedPower(); }
int Blueprint::GetFixedComputer() const { return m_pDef->GetFixedComputer(); }

int Blueprint::GetInitiative() const
{
	int n = GetFixedInitiative();
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetInitiative(s);
	return n;
}

int Blueprint::GetPowerSource() const
{
	int n = GetFixedPower();
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetPowerSource(s);
	return n;
}

int Blueprint::GetPowerDrain() const
{
	int n = 0;
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetPowerDrain(s);
	return n;
}

int Blueprint::GetComputer() const
{
	int n = GetFixedComputer();
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetComputer(s);
	return n;
}

int Blueprint::GetMovement() const
{
	int n = 0;
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetMovement(s);
	return n;
}

int Blueprint::GetHulls() const
{
	int n = 0;
	for (auto s : SlotRange(*this))
		n += ShipLayout::GetHulls(s);
	return n;
}

ShipPart Blueprint::GetSlot(int i) const
{
	ShipPart overlay = m_overlay.GetSlot(i);
	return overlay == ShipPart::Empty ? m_pDef->GetBaseLayout().GetSlot(i) : overlay;
}

const Blueprint& Blueprint::GetAncientShip()
{
	static Blueprint blueprint(BlueprintDefs::GetAncientShip());
	return blueprint;
}

const Blueprint& Blueprint::GetGCDS()
{
	static Blueprint blueprint(BlueprintDefs::GCDS());
	return blueprint;
}

void Blueprint::Save(Serial::SaveNode& node) const
{
	node.SaveClass("overlay", m_overlay);
}

void Blueprint::Load(const Serial::LoadNode& node)
{
	node.LoadClass("overlay", m_overlay);
}
