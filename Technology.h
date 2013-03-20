#pragma once

enum class TechType {	
	NeutronBomb, StarBase,	   PlasmaCannon, PhaseShield,  AdvMining,  TachyonSource, PlasmaMissile,    GluonComp,
	GaussShield, ImprovedHull, FusionSource, PositronComp, AdvEconomy, TachyonDrive,  AntimatterCannon, QuantumGrid,
	NanoRobots,	 FusionDrive,  AdvRobotics,  Orbital,      AdvLabs,    Monolith,      ArtifactKey,      WormholeGen  };

	class Technology
{
public:
	enum class Class { Military, Grid, Nano, _Count };

	Technology(TechType t);
	~Technology();

	int GetTier() const;
	Class GetClass() const;
	
	static int GetMaxCost(int tier);
	static int GetMinCost(int tier);

private:
	TechType m_type;
};

