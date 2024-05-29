#include "Configs.h"
#include "Globals.h"
#include "SimpleIni.h"
#include "Utils.h"
#include "Types.h"
#include <fstream>

std::unordered_map<RE::TESAmmo*, bool> Configs::ammoWhitelist;
std::vector<RE::BGSProjectile*> Configs::EFDProjectiles;
size_t Configs::maxTier;
std::vector<uint16_t> Configs::helmetRatings;
std::vector<uint16_t> Configs::vestRatings;
float Configs::headFatalityDecPerTier;
float Configs::torsoFatalityDecPerTier;
std::unordered_map<std::string, CartridgeData> Configs::cartridgeData;
std::unordered_map<EFDBodyParts, BleedingData> Configs::bleedingConfigs;
GlobalData Configs::gdHead;
GlobalData Configs::gdTorso;
FatalityIncrement Configs::fi;
std::vector<RE::TESRace*> Configs::deathMarkExcludedList;
bool Configs::playerForceKill;

std::unordered_map<RE::TESAmmo*, bool> GetAmmunitionWhitelist()
{
	std::unordered_map<RE::TESAmmo*, bool> ret;

	//게임에 로드된 폼 중 탄약만 가져온다
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::TESAmmo*> ammos = dh->GetFormArray<RE::TESAmmo>();

	//모든 탄약 폼에 대하여
	for (auto it = ammos.begin(); it != ammos.end(); ++it) {
		RE::TESAmmo* ammo = *it;
		if (ammo->data.damage > 0 && ammo->data.projectile && ammo->data.projectile->data.flags & 0x10000) {
			logger::info("{} is DeBadFall certified.", ammo->fullName.c_str());
			ret.insert(std::pair<RE::TESAmmo*, bool>(ammo, true));
		}
	}
	return ret;
}

void SetupWeapons(CSimpleIniA& ini)
{
	uint16_t uniqueDmg = (uint16_t)std::stoi(ini.GetValue("General", "UniqueDamage", "15"));
	logger::info("UniqueDamage {}", uniqueDmg);

	//탄약 화이트리스트 제작
	Configs::ammoWhitelist = GetAmmunitionWhitelist();

	//바닐라에서 총과 유니크 무기의 정의를 가져옴
	RE::BGSListForm* gunsKeywordList = (RE::BGSListForm*)RE::TESForm::GetFormByID(0xF78EC);
	RE::BGSKeyword* uniqueKeyword = (RE::BGSKeyword*)RE::TESForm::GetFormByID(0x1B3FAC);

	logger::info("Keyword list found.");
	//게임에 로드된 모든 무기 폼에 대하여
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::TESObjectWEAP*> weapons = dh->GetFormArray<RE::TESObjectWEAP>();
	for (auto it = weapons.begin(); it != weapons.end(); ++it) {
		RE::TESObjectWEAP* wep = (*it);
		if (wep->keywords) {
			for (auto lit = gunsKeywordList->arrayOfForms.begin(); lit != gunsKeywordList->arrayOfForms.end(); ++lit) {
				//무기가 총기로 확인되면
				if (wep->HasKeyword(static_cast < RE::BGSKeyword * >(*lit))) {
					//탄약 화이트리스트와 대조
					RE::TESAmmo* ammo = wep->weaponData.ammo;
					if (ammo && Configs::ammoWhitelist.find(ammo) != Configs::ammoWhitelist.end()) {
						logger::info("Weapon Found : {} (FormID {:08x} at {:08x})", wep->fullName.c_str(), wep->formID, (uintptr_t)wep);
						uint16_t oldDamage = wep->weaponData.attackDamage;
						//유니크 확인
						bool isUnique = false;
						if (wep->HasKeyword(uniqueKeyword)) {
							isUnique = true;
						}
						wep->weaponData.attackDamage = isUnique * uniqueDmg;
						logger::info("Old dmg {}, New dmg 0", oldDamage);
					}
				}
			}
		}
	}
}

void SetupArmors(CSimpleIniA& ini)
{
	CSimpleIniA::TNamesDepend helmetKeys;
	ini.GetAllKeys("Helmet", helmetKeys);
	helmetKeys.sort(CSimpleIniA::Entry::LoadOrder());
	logger::info("Helmet Tier References");
	Configs::helmetRatings.clear();
	for (auto key = helmetKeys.begin(); key != helmetKeys.end(); ++key) {
		uint16_t ref = (uint16_t)std::stoi(ini.GetValue("Helmet", key->pItem));
		Configs::helmetRatings.push_back(ref);
		logger::info("{}", ref);
	}

	CSimpleIniA::TNamesDepend vestKeys;
	ini.GetAllKeys("Vest", vestKeys);
	vestKeys.sort(CSimpleIniA::Entry::LoadOrder());
	logger::info("Vest Tier References");
	Configs::vestRatings.clear();
	for (auto key = vestKeys.begin(); key != vestKeys.end(); ++key) {
		uint16_t ref = (uint16_t)std::stoi(ini.GetValue("Vest", key->pItem));
		Configs::vestRatings.push_back(ref);
		logger::info("{}", ref);
	}
	Configs::maxTier = max(Configs::helmetRatings.size(), Configs::vestRatings.size());
	logger::info("Tier max : {}", Configs::maxTier);
}

void SetupDeathMark(CSimpleIniA& ini)
{
	Configs::playerForceKill = std::stoi(ini.GetValue("General", "PlayerForceKill")) > 0;
	logger::info("PlayerForceKill {}", Configs::playerForceKill);
	logger::info("Laser");
	std::vector<float> laserConditions;
	float headChance = std::stof(ini.GetValue("CartridgeFatalities", "LaserHead"));
	logger::info("Head {}", headChance);
	float torsoChance = std::stof(ini.GetValue("CartridgeFatalities", "LaserTorso"));
	logger::info("Torso {}", torsoChance);
	laserConditions.push_back(headChance);
	laserConditions.push_back(torsoChance);
	CSimpleIniA::TNamesDepend protectionKeys;
	ini.GetAllKeys("Laser", protectionKeys);
	protectionKeys.sort(CSimpleIniA::Entry::LoadOrder());
	std::vector<uint16_t> laserChances;
	for (auto key = protectionKeys.begin(); key != protectionKeys.end(); ++key) {
		laserChances.push_back((uint16_t)std::stoi((ini.GetValue("Laser", key->pItem))));
		logger::info("{}", std::stoi((ini.GetValue("Laser", key->pItem))));
	}
	if (laserChances.size() < Configs::maxTier) {
		logger::info("Cartridge {} is missing some data! Auto filling...", "Laser");
		if (laserChances.size() == 0) {
			for (int i = 0; i < Configs::maxTier; ++i) {
				laserChances.push_back(0);
			}
		} else {
			for (int i = 0; i < Configs::maxTier - laserChances.size(); ++i) {
				laserChances.push_back(laserChances[laserChances.size() - 1]);
				logger::info("{}", laserChances[laserChances.size() - 1]);
			}
		}
	}
	CartridgeData laserCd;
	laserCd.deathConditions = laserConditions;
	laserCd.protectionChances = laserChances;
	Configs::cartridgeData.insert(std::pair<std::string, CartridgeData>(std::string("Laser"), laserCd));

	logger::info("Physical Projectiles");
	headChance = std::stof(ini.GetValue("CartridgeFatalities", "PhysicalHead"));
	logger::info("Head {}", headChance);
	Configs::gdHead.formulaA = std::stof(ini.GetValue("PhysicalHead", "FormulaA"));
	Configs::gdHead.formulaB = std::stof(ini.GetValue("PhysicalHead", "FormulaB"));
	Configs::gdHead.formulaC = std::stof(ini.GetValue("PhysicalHead", "FormulaC"));
	Configs::gdHead.formulaD = std::stof(ini.GetValue("PhysicalHead", "FormulaD"));
	Configs::gdHead.PrintData();
	torsoChance = std::stof(ini.GetValue("CartridgeFatalities", "PhysicalTorso"));
	logger::info("Torso {}", torsoChance);
	Configs::gdTorso.formulaA = std::stof(ini.GetValue("PhysicalTorso", "FormulaA"));
	Configs::gdTorso.formulaB = std::stof(ini.GetValue("PhysicalTorso", "FormulaB"));
	Configs::gdTorso.formulaC = std::stof(ini.GetValue("PhysicalTorso", "FormulaC"));
	Configs::gdTorso.formulaD = std::stof(ini.GetValue("PhysicalTorso", "FormulaD"));
	Configs::gdTorso.PrintData();

	Configs::fi.formulaA = std::stof(ini.GetValue("FatalityIncrement", "FormulaA"));
	Configs::fi.formulaB = std::stof(ini.GetValue("FatalityIncrement", "FormulaB"));
	Configs::fi.formulaC = std::stof(ini.GetValue("FatalityIncrement", "FormulaC"));
	Configs::fi.PrintData();

	Configs::headFatalityDecPerTier = std::stof(ini.GetValue("CartridgeFatalities", "HeadFatalityDecreasePerTier"));
	logger::info("HeadFatalityDecreasePerTier {}", Configs::headFatalityDecPerTier);
	Configs::torsoFatalityDecPerTier = std::stof(ini.GetValue("CartridgeFatalities", "TorsoFatalityDecreasePerTier"));
	logger::info("TorsoFatalityDecreasePerTier {}", Configs::torsoFatalityDecPerTier);

	std::vector<float> globalConditions;
	globalConditions.push_back(headChance);
	globalConditions.push_back(torsoChance);
	std::vector<uint16_t> globalChances;
	for (int i = 0; i < Configs::maxTier; ++i) {
		globalChances.push_back(0);
	}
	CartridgeData globalCd;
	globalCd.deathConditions = globalConditions;
	globalCd.protectionChances = globalChances;
	Configs::cartridgeData.insert(std::pair<std::string, CartridgeData>(std::string("Global"), globalCd));
}

void SetupBleeding(CSimpleIniA& ini)
{
	BleedingData bldHead;
	bldHead.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed Head"));
	bldHead.conditionStart = std::stof(ini.GetValue("BleedingHead", "ConditionStart"));
	bldHead.conditionThreshold = std::stof(ini.GetValue("BleedingHead", "ConditionThreshold"));
	bldHead.chanceMin = std::stof(ini.GetValue("BleedingHead", "ChanceMin"));
	bldHead.chanceMax = std::stof(ini.GetValue("BleedingHead", "ChanceMax"));
	bldHead.initialDamage = std::stof(ini.GetValue("BleedingHead", "InitialDamage"));
	bldHead.multiplier = std::stof(ini.GetValue("BleedingHead", "Multiplier"));
	bldHead.duration = std::stof(ini.GetValue("BleedingHead", "Duration"));
	//골절
	bldHead.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture Dummy"));
	bldHead.fractureChance = std::stof(ini.GetValue("BleedingHead", "FractureChance"));

	for (auto it = bldHead.spell->listOfEffects.begin(); it != bldHead.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldHead.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldHead.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::Head, bldHead));
	logger::info("_SpellConditionBleedHead {:08X}", (uintptr_t)bldHead.spell);
	logger::info("_SpellConditionFractureHead {:08X}", (uintptr_t)bldHead.fractureSpell);
	bldHead.PrintData();

	BleedingData bldTorso;
	bldTorso.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed Torso"));
	bldTorso.conditionStart = std::stof(ini.GetValue("BleedingTorso", "ConditionStart"));
	bldTorso.conditionThreshold = std::stof(ini.GetValue("BleedingTorso", "ConditionThreshold"));
	bldTorso.chanceMin = std::stof(ini.GetValue("BleedingTorso", "ChanceMin"));
	bldTorso.chanceMax = std::stof(ini.GetValue("BleedingTorso", "ChanceMax"));
	bldTorso.initialDamage = std::stof(ini.GetValue("BleedingTorso", "InitialDamage"));
	bldTorso.multiplier = std::stof(ini.GetValue("BleedingTorso", "Multiplier"));
	bldTorso.duration = std::stof(ini.GetValue("BleedingTorso", "Duration"));
	//골절
	bldTorso.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture Dummy"));
	bldTorso.fractureChance = std::stof(ini.GetValue("BleedingTorso", "FractureChance"));

	for (auto it = bldTorso.spell->listOfEffects.begin(); it != bldTorso.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldTorso.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldTorso.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::Torso, bldTorso));
	logger::info("_SpellConditionBleedTorso {:08X}", (uintptr_t)bldTorso.spell);
	logger::info("_SpellConditionFractureTorso {:08X}", (uintptr_t)bldTorso.fractureSpell);
	bldTorso.PrintData();

	BleedingData bldLArm;
	bldLArm.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed LArm"));
	bldLArm.conditionStart = std::stof(ini.GetValue("BleedingArm", "ConditionStart"));
	bldLArm.conditionThreshold = std::stof(ini.GetValue("BleedingArm", "ConditionThreshold"));
	bldLArm.chanceMin = std::stof(ini.GetValue("BleedingArm", "ChanceMin"));
	bldLArm.chanceMax = std::stof(ini.GetValue("BleedingArm", "ChanceMax"));
	bldLArm.initialDamage = std::stof(ini.GetValue("BleedingArm", "InitialDamage"));
	bldLArm.multiplier = std::stof(ini.GetValue("BleedingArm", "Multiplier"));
	bldLArm.duration = std::stof(ini.GetValue("BleedingArm", "Duration"));
	//골절
	bldLArm.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture LArm"));
	bldLArm.fractureChance = std::stof(ini.GetValue("BleedingArm", "FractureChance"));

	for (auto it = bldLArm.spell->listOfEffects.begin(); it != bldLArm.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldLArm.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldLArm.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::LArm, bldLArm));
	logger::info("_SpellConditionBleedLArm {:08X}", (uintptr_t)bldLArm.spell);
	logger::info("_SpellConditionFractureLArm {:08X}", (uintptr_t)bldLArm.fractureSpell);
	bldLArm.PrintData();

	BleedingData bldLLeg;
	bldLLeg.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed LLeg"));
	bldLLeg.conditionStart = std::stof(ini.GetValue("BleedingLeg", "ConditionStart"));
	bldLLeg.conditionThreshold = std::stof(ini.GetValue("BleedingLeg", "ConditionThreshold"));
	bldLLeg.chanceMin = std::stof(ini.GetValue("BleedingLeg", "ChanceMin"));
	bldLLeg.chanceMax = std::stof(ini.GetValue("BleedingLeg", "ChanceMax"));
	bldLLeg.initialDamage = std::stof(ini.GetValue("BleedingLeg", "InitialDamage"));
	bldLLeg.multiplier = std::stof(ini.GetValue("BleedingLeg", "Multiplier"));
	bldLLeg.duration = std::stof(ini.GetValue("BleedingLeg", "Duration"));
	//골절
	bldLLeg.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture LLeg"));
	bldLLeg.fractureChance = std::stof(ini.GetValue("BleedingLeg", "FractureChance"));

	for (auto it = bldLLeg.spell->listOfEffects.begin(); it != bldLLeg.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldLLeg.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldLLeg.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::LLeg, bldLLeg));
	logger::info("_SpellConditionBleeddLLeg {:08X}", (uintptr_t)bldLLeg.spell);
	logger::info("_SpellConditionFracturedLLeg {:08X}", (uintptr_t)bldLLeg.fractureSpell);
	bldLLeg.PrintData();

	BleedingData bldRArm;
	bldRArm.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed RArm"));
	bldRArm.conditionStart = std::stof(ini.GetValue("BleedingArm", "ConditionStart"));
	bldRArm.conditionThreshold = std::stof(ini.GetValue("BleedingArm", "ConditionThreshold"));
	bldRArm.chanceMin = std::stof(ini.GetValue("BleedingArm", "ChanceMin"));
	bldRArm.chanceMax = std::stof(ini.GetValue("BleedingArm", "ChanceMax"));
	bldRArm.initialDamage = std::stof(ini.GetValue("BleedingArm", "InitialDamage"));
	bldRArm.multiplier = std::stof(ini.GetValue("BleedingArm", "Multiplier"));
	bldRArm.duration = std::stof(ini.GetValue("BleedingArm", "Duration"));
	//골절
	bldRArm.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture RArm"));
	bldRArm.fractureChance = std::stof(ini.GetValue("BleedingArm", "FractureChance"));

	for (auto it = bldRArm.spell->listOfEffects.begin(); it != bldRArm.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldRArm.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldRArm.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::RArm, bldRArm));
	logger::info("_SpellConditionBleedRArm {:08X}", (uintptr_t)bldRArm.spell);
	logger::info("_SpellConditionFractureRArm {:08X}", (uintptr_t)bldRArm.fractureSpell);
	bldRArm.PrintData();

	BleedingData bldRLeg;
	bldRLeg.spell = Utils::GetSpellByFullName(std::string("EFD Condition Bleed RLeg"));
	bldRLeg.conditionStart = std::stof(ini.GetValue("BleedingLeg", "ConditionStart"));
	bldRLeg.conditionThreshold = std::stof(ini.GetValue("BleedingLeg", "ConditionThreshold"));
	bldRLeg.chanceMin = std::stof(ini.GetValue("BleedingLeg", "ChanceMin"));
	bldRLeg.chanceMax = std::stof(ini.GetValue("BleedingLeg", "ChanceMax"));
	bldRLeg.initialDamage = std::stof(ini.GetValue("BleedingLeg", "InitialDamage"));
	bldRLeg.multiplier = std::stof(ini.GetValue("BleedingLeg", "Multiplier"));
	bldRLeg.duration = std::stof(ini.GetValue("BleedingLeg", "Duration"));
	//골절
	bldRLeg.fractureSpell = Utils::GetSpellByFullName(std::string("EFD Condition Fracture RLeg"));
	bldRLeg.fractureChance = std::stof(ini.GetValue("BleedingLeg", "FractureChance"));

	for (auto it = bldRLeg.spell->listOfEffects.begin(); it != bldRLeg.spell->listOfEffects.end(); ++it) {
		(*it)->data.magnitude = bldRLeg.initialDamage;
		(*it)->data.duration = (int32_t)floor(bldRLeg.duration);
	}
	Configs::bleedingConfigs.insert(std::pair<EFDBodyParts, BleedingData>(EFDBodyParts::RLeg, bldRLeg));
	logger::info("_SpellConditionBleedRLeg {:08X}", (uintptr_t)bldRLeg.spell);
	logger::info("_SpellConditionFractureRLeg {:08X}", (uintptr_t)bldRLeg.fractureSpell);
	bldRLeg.PrintData();
}

void Configs::LoadConfigs()
{
	std::string path = "Data\\F4SE\\Plugins\\DamageSystem.ini";
	CSimpleIniA ini(true, false, false);
	SI_Error result = ini.LoadFile(path.c_str());
	if (result >= 0) {
		SetupWeapons(ini);
		SetupArmors(ini);
		SetupDeathMark(ini);
		SetupBleeding(ini);
	}
	ini.Reset();
}
