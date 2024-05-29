#pragma once

namespace Globals {
	extern RE::PlayerCharacter* p;
	extern RE::ActorValueInfo* helmetTier;
	extern RE::ActorValueInfo* vestTier;
	extern RE::ActorValueInfo* lasthitpart;
	extern RE::ActorValueInfo* perceptionCondition;
	extern RE::ActorValueInfo* enduranceCondition;
	extern RE::ActorValueInfo* leftAttackCondition;
	extern RE::ActorValueInfo* rightAttackCondition;
	extern RE::ActorValueInfo* leftMobilityCondition;
	extern RE::ActorValueInfo* rightMobilityCondition;
	extern RE::ActorValueInfo* brainCondition;
	extern RE::ActorValueInfo* health;
	extern RE::SpellItem* killDeathMarked;
	extern RE::BGSSoundDescriptorForm* deathMarkHeadSound;
	extern RE::BGSSoundDescriptorForm* deathMarkTorsoSound;
	extern RE::BGSSoundDescriptorForm* avoidedDeathSound;
	extern RE::BGSSoundDescriptorForm* avoidedDeathBuzzing;
	extern RE::TESImageSpaceModifier* avoidedDeathIMOD;
	extern RE::TESForm* dtPhysical;
	extern RE::ActorValueInfo** EFDConditions[];
	extern std::string EFDBodyPartsName[];
	extern std::vector<RE::BGSBodyPartData::PartType> torsoParts;
	extern std::vector<RE::BGSBodyPartData::PartType> headParts;
	extern std::vector<RE::BGSBodyPartData::PartType> larmParts;
	extern std::vector<RE::BGSBodyPartData::PartType> llegParts;
	extern std::vector<RE::BGSBodyPartData::PartType> rarmParts;
	extern std::vector<RE::BGSBodyPartData::PartType> rlegParts;
	extern std::vector<RE::TESRace*> deathMarkExcludedList;
	extern float lastDeathMarkSoundTime;
	extern float lastAvoidedDeathBuzzingTime;

	void InitializeGlobals();
}
