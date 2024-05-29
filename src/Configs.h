#pragma once
#include <string>
#include "Types.h"

namespace Configs {
	extern std::unordered_map<RE::TESAmmo*, bool> ammoWhitelist;
	extern std::vector<RE::BGSProjectile*> EFDProjectiles;
	extern size_t maxTier;
	extern std::vector<uint16_t> helmetRatings;
	extern std::vector<uint16_t> vestRatings;
	extern float headFatalityDecPerTier;
	extern float torsoFatalityDecPerTier;
	extern std::unordered_map<std::string, CartridgeData> cartridgeData;
	extern std::unordered_map<EFDBodyParts, BleedingData> bleedingConfigs;
	extern GlobalData gdHead;
	extern GlobalData gdTorso;
	extern FatalityIncrement fi;
	extern std::vector<RE::TESRace*> deathMarkExcludedList;
	extern bool playerForceKill;
	void LoadConfigs();
}
