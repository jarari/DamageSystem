#pragma once
#include <vector>
#include <string>

struct CartridgeData
{
	std::vector<uint16_t> protectionChances;
	std::vector<float> deathConditions;
	float fatalityIncrement;
};

enum EFDBodyParts
{
	None,
	Head,
	Torso,
	LArm,
	LLeg,
	RArm,
	RLeg
};

struct BleedingData
{
	RE::SpellItem* spell;
	RE::SpellItem* fractureSpell;
	float conditionStart;
	float conditionThreshold;
	float chanceMin;
	float chanceMax;
	float initialDamage;
	float multiplier;
	float duration;
	float fractureChance;
	void PrintData()
	{
		logger::info("Condition Start {}", conditionStart);
		logger::info("Condition Threshold {}", conditionThreshold);
		logger::info("Chance Min {}", chanceMin);
		logger::info("Chance Max {}", chanceMax);
		logger::info("Initial Damage {}", initialDamage);
		logger::info("Multiplier {}", multiplier);
		logger::info("Duration {}", duration);
		logger::info("Fracture Chance {}", fractureChance);
	}
};

//즉사 공식, 방어력 데미지 설정
struct GlobalData
{
	float formulaA;
	float formulaB;
	float formulaC;
	float formulaD;
	void PrintData()
	{
		logger::info("FormulaA {}", formulaA);
		logger::info("FormulaB {}", formulaB);
		logger::info("FormulaC {}", formulaC);
		logger::info("FormulaD {}", formulaD);
	}
};

//즉사 커트라인 조정
struct FatalityIncrement
{
	float formulaA;
	float formulaB;
	float formulaC;
	void PrintData()
	{
		logger::info("FormulaA {}", formulaA);
		logger::info("FormulaB {}", formulaB);
		logger::info("FormulaC {}", formulaC);
	}
};
