#include "Globals.h"
#include "Utils.h"

RE::PlayerCharacter* Globals::p = nullptr;
RE::ActorValueInfo** Globals::EFDConditions[] = {
	nullptr,
	&perceptionCondition,
	&enduranceCondition,
	&leftAttackCondition,
	&leftMobilityCondition,
	&rightAttackCondition,
	&rightMobilityCondition
};

std::string Globals::EFDBodyPartsName[] = {
	"None",
	"Head",
	"Torso",
	"LArm",
	"LLeg",
	"RArm",
	"RLeg"
};
std::vector<RE::BGSBodyPartData::PartType> Globals::torsoParts = { RE::BGSBodyPartData::PartType::COM, RE::BGSBodyPartData::PartType::Torso, RE::BGSBodyPartData::PartType::Pelvis };
std::vector<RE::BGSBodyPartData::PartType> Globals::headParts = { RE::BGSBodyPartData::PartType::Eye, RE::BGSBodyPartData::PartType::Head1, RE::BGSBodyPartData::PartType::Head2, RE::BGSBodyPartData::PartType::Brain };
std::vector<RE::BGSBodyPartData::PartType> Globals::larmParts = { RE::BGSBodyPartData::PartType::LeftArm1 };
std::vector<RE::BGSBodyPartData::PartType> Globals::llegParts = { RE::BGSBodyPartData::PartType::LeftLeg1, RE::BGSBodyPartData::PartType::LeftFoot };
std::vector<RE::BGSBodyPartData::PartType> Globals::rarmParts = { RE::BGSBodyPartData::PartType::RightArm1, RE::BGSBodyPartData::PartType::Weapon };
std::vector<RE::BGSBodyPartData::PartType> Globals::rlegParts = { RE::BGSBodyPartData::PartType::RightLeg1, RE::BGSBodyPartData::PartType::RightFoot };
std::vector<RE::TESRace*> Globals::deathMarkExcludedList = {};
RE::ActorValueInfo* Globals::helmetTier;
RE::ActorValueInfo* Globals::vestTier;
RE::ActorValueInfo* Globals::lasthitpart;
RE::ActorValueInfo* Globals::perceptionCondition;
RE::ActorValueInfo* Globals::enduranceCondition;
RE::ActorValueInfo* Globals::leftAttackCondition;
RE::ActorValueInfo* Globals::rightAttackCondition;
RE::ActorValueInfo* Globals::leftMobilityCondition;
RE::ActorValueInfo* Globals::rightMobilityCondition;
RE::ActorValueInfo* Globals::brainCondition;
RE::ActorValueInfo* Globals::health;
RE::SpellItem* Globals::killDeathMarked;
RE::BGSSoundDescriptorForm* Globals::deathMarkHeadSound;
RE::BGSSoundDescriptorForm* Globals::deathMarkTorsoSound;
RE::BGSSoundDescriptorForm* Globals::avoidedDeathSound;
RE::BGSSoundDescriptorForm* Globals::avoidedDeathBuzzing;
RE::TESImageSpaceModifier* Globals::avoidedDeathIMOD;
RE::TESForm* Globals::dtPhysical;
float Globals::lastDeathMarkSoundTime;
float Globals::lastAvoidedDeathBuzzingTime;

void Globals::InitializeGlobals() {
	p = RE::PlayerCharacter::GetSingleton();

	helmetTier = Utils::GetAVIFByEditorID("EFD_Helmet_Tier");
	vestTier = Utils::GetAVIFByEditorID("EFD_Vest_Tier");
	dtPhysical = RE::TESForm::GetFormByID(0x60A87);
	logger::info("EFD_Helmet_Tier {:08X}", (uintptr_t)Globals::helmetTier);
	logger::info("EFD_Vest_Tier {:08X}", (uintptr_t)Globals::vestTier);

	lasthitpart = Utils::GetAVIFByEditorID("EFD_LastHitPart");
	health = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x0002D4);
	perceptionCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x00036C);
	enduranceCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x00036D);
	leftAttackCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x00036E);
	rightAttackCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x00036F);
	leftMobilityCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x000370);
	rightMobilityCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x000371);
	brainCondition = (RE::ActorValueInfo*)RE::TESForm::GetFormByID(0x000372);

	killDeathMarked = Utils::GetSpellByFullName(std::string("EFD Kill DeathMarked"));

	RE::BGSKeyword* actorTypeLibertyPrime = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0x10D529);
	RE::BGSKeyword* actorTypeMirelurk = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0x2CB71);
	RE::BGSKeyword* actorTypeRobot = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0x2CB73);
	RE::BGSKeyword* actorTypeTurret = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0xB2BF3);
	RE::BGSKeyword* isVertibird = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0xF9899);
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::TESRace*> races = dh->GetFormArray<RE::TESRace>();
	for (auto it = races.begin(); it != races.end(); ++it) {
		RE::BGSKeyword** keywords = (*it)->keywords;
		uint32_t i = 0;
		while (i < (*it)->numKeywords) {
			if (keywords[i] == actorTypeLibertyPrime || keywords[i] == actorTypeMirelurk || keywords[i] == actorTypeRobot || keywords[i] == actorTypeTurret || keywords[i] == isVertibird) {
				deathMarkExcludedList.push_back(*it);
				i = (*it)->numKeywords;
				logger::info("{} excluded from headshot", (*it)->GetFullName());
			}
			++i;
		}
	}

	deathMarkHeadSound = (RE::BGSSoundDescriptorForm*)Utils::GetFormFromMod("WB - EFD Damage System.esp", 0x817);
	deathMarkTorsoSound = (RE::BGSSoundDescriptorForm*)Utils::GetFormFromMod("WB - EFD Damage System.esp", 0x818);
	avoidedDeathSound = (RE::BGSSoundDescriptorForm*)Utils::GetFormFromMod("WB - EFD Damage System.esp", 0x819);
	avoidedDeathIMOD = (RE::TESImageSpaceModifier*)Utils::GetFormFromMod("WB - EFD Damage System.esp", 0x872);
	avoidedDeathBuzzing = (RE::BGSSoundDescriptorForm*)Utils::GetFormFromMod("WB - EFD Damage System.esp", 0x873);
}
