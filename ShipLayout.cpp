#include "stdafx.h"
#include "ShipLayout.h"
#include "Ship.h"
#include "EnumTraits.h"
#include "Serial.h"

ShipLayout::ShipLayout() : m_type(ShipType::None)
{
}

ShipLayout::ShipLayout(ShipType type) : m_type(ShipType::None)
{
	SetType(type);
}

void ShipLayout::SetType(ShipType t) 
{
	ASSERT(m_slots.empty());
	m_type = t;
	m_slots.resize(GetSlotCount(t));
}

int ShipLayout::GetInitiative(ShipPart p)
{
	switch (p)
	{
		case ShipPart::GluonComp:
		case ShipPart::NuclearDrive:	return 1;
		case ShipPart::PositronComp:
		case ShipPart::TachyonDrive:
		case ShipPart::ConformalDrive:	return 2;
		case ShipPart::FusionDrive:		return 3;
	}
	return 0;
}

int ShipLayout::GetPowerSource(ShipPart p)
{
	switch (p)
	{
	case ShipPart::NuclearSource:	return 3;
	case ShipPart::TachyonSource:	return 6;
	case ShipPart::FusionSource:	return 9;
	case ShipPart::HypergridSource:	return 11;
	}
	return 0;
}

int ShipLayout::GetPowerDrain(ShipPart p)
{
	switch (p)
	{
	case ShipPart::GluonComp:
	case ShipPart::IonCannon:
	case ShipPart::GaussShield:
	case ShipPart::NuclearDrive:
	case ShipPart::IonTurret:			return 1;
	case ShipPart::PositronComp:
	case ShipPart::TachyonDrive:
	case ShipPart::PlasmaCannon:
	case ShipPart::ConformalDrive:
	case ShipPart::FluxShield:			return 2;
	case ShipPart::FusionDrive:			return 3;
	case ShipPart::AntimatterCannon:	return 4;
	}
	return 0;
}

int ShipLayout::GetComputer(ShipPart p)
{
	switch (p)
	{
	case ShipPart::ElectronComp:	return 1;
	case ShipPart::GluonComp:		return 2;
	case ShipPart::AxionComp:
	case ShipPart::PositronComp:	return 3;
	}
	return 0;
}

int ShipLayout::GetShield(ShipPart p)
{
	switch (p)
	{
	case ShipPart::PhaseShield:	return 1;
	case ShipPart::GaussShield:	return 2;
	case ShipPart::FluxShield:	return 3;
	}
	return 0;
}

int ShipLayout::GetMovement(ShipPart p)
{
	switch (p)
	{
	case ShipPart::NuclearDrive:	return 1;
	case ShipPart::TachyonDrive:	return 2;
	case ShipPart::FusionDrive:		return 3;
	case ShipPart::ConformalDrive:	return 4;
	}
	return 0;
}

int ShipLayout::GetHulls(ShipPart p)
{
	switch (p)
	{
	case ShipPart::Hull:			return 1;
	case ShipPart::ImprovedHull :	return 2;
	}
	return 0;
}

int ShipLayout::GetSlotCount(ShipType t) 
{
	switch (t)
	{
	case ShipType::Interceptor:	return 4;
	case ShipType::Starbase:	return 5;
	case ShipType::Cruiser:		return 6;
	case ShipType::Dreadnought:	return 8;
	case ShipType::Ancient:		return 3;
	case ShipType::GCDS:		return 11;
	}
	ASSERT(false);
	return 0;
}

void ShipLayout::Save(Serial::SaveNode& node) const
{
	node.SaveEnum("type", m_type); 
	node.SaveCntr("slots", m_slots, Serial::EnumSaver());
}

void ShipLayout::Load(const Serial::LoadNode& node)
{
	node.LoadEnum("type", m_type); 
	node.LoadCntr("slots", m_slots, Serial::EnumLoader());
}

DEFINE_ENUM_NAMES(ShipPart) {	"Empty", "Blocked", 
								"IonCannon", "PlasmaCannon", "AntimatterCannon", 
								"PlasmaMissile", "IonMissile",
								"IonTurret",
								"ElectronComp", "GluonComp", "PositronComp", "AxionComp",
								"NuclearDrive", "TachyonDrive", "FusionDrive", "ConformalDrive"
								"NuclearSource", "TachyonSource", "FusionSource", "HypergridSource",
								"PhaseShield", "GaussShield", "FluxShield",
								"Hull", "ImprovedHull", "" };
