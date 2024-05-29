#include "Utils.h"
#include <Configs.h>
#include <Globals.h>

REL::Relocation<uint32_t*> Utils::ptr_invalidhandle{ REL::ID(888641) };
REL::Relocation<float*> Utils::ptr_engineTime{ REL::ID(599343) };
REL::Relocation<float*> Utils::ptr_deltaTime{ REL::ID(1013228) };

RE::TESForm* Utils::GetFormFromMod(std::string modname, uint32_t formid) {
	if (!modname.length() || !formid)
		return nullptr;
	return RE::TESDataHandler::GetSingleton()->LookupForm(formid, modname);
}

RE::ActorValueInfo* Utils::GetAVIFByEditorID(std::string editorID)
{
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::ActorValueInfo*> avifs = dh->GetFormArray<RE::ActorValueInfo>();
	for (auto it = avifs.begin(); it != avifs.end(); ++it) {
		if (strcmp((*it)->formEditorID.c_str(), editorID.c_str()) == 0) {
			return (*it);
		}
	}
	return nullptr;
}

RE::BGSExplosion* Utils::GetExplosionByFullName(std::string explosionname)
{
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::BGSExplosion*> explosions = dh->GetFormArray<RE::BGSExplosion>();
	for (auto it = explosions.begin(); it != explosions.end(); ++it) {
		if (strcmp((*it)->GetFullName(), explosionname.c_str()) == 0) {
			return (*it);
		}
	}
	return nullptr;
}

RE::SpellItem* Utils::GetSpellByFullName(std::string spellname)
{
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::SpellItem*> spells = dh->GetFormArray<RE::SpellItem>();
	for (auto it = spells.begin(); it != spells.end(); ++it) {
		if (strcmp((*it)->GetFullName(), spellname.c_str()) == 0) {
			return (*it);
		}
	}
	return nullptr;
}

RE::EffectSetting* Utils::GetMagicEffectByFullName(std::string effectname)
{
	RE::TESDataHandler* dh = RE::TESDataHandler::GetSingleton();
	RE::BSTArray<RE::EffectSetting*> mgefs = dh->GetFormArray<RE::EffectSetting>();
	for (auto it = mgefs.begin(); it != mgefs.end(); ++it) {
		if (strcmp((*it)->GetFullName(), effectname.c_str()) == 0) {
			return (*it);
		}
	}
	return nullptr;
}

bool Utils::IsInPA(RE::Actor* a){
	if (!a || !a->extraList) {
		return false;
	}
	return a->extraList->HasType(RE::EXTRA_DATA_TYPE::kPowerArmor);
}

//*********************Biped Slots********************
// 30	-	0x1
// 31	-	0x2
// 32	-	0x4
// 33	-	0x8
// 34	-	0x10
// 35	-	0x20
// 36	-	0x40
// 37	-	0x80
// 38	-	0x100
// 39	-	0x200
// 40	-	0x400
// 41	-	0x800
// 42	-	0x1000
// 43	-	0x2000
// 44	-	0x4000
// 45	-	0x8000
//****************************************************

void SetHelmetTier(RE::Actor* a, uint16_t rating) {
	size_t i = Configs::helmetRatings.size() - 1;
	bool intervalFound = false;
	while (i >= 0 && !intervalFound) {
		if (rating >= Configs::helmetRatings[i]) {
			a->SetActorValue(*Globals::helmetTier, i);
			intervalFound = true;
		}
		--i;
	}
	if (!intervalFound) {
		a->SetActorValue(*Globals::helmetTier, 0);
	}
}

void SetVestTier(RE::Actor* a, uint16_t rating)
{
	size_t i = Configs::vestRatings.size() - 1;
	bool intervalFound = false;
	while (i >= 0 && !intervalFound) {
		if (rating >= Configs::vestRatings[i]) {
			a->SetActorValue(*Globals::vestTier, i);
			intervalFound = true;
		}
		--i;
	}
	if (!intervalFound) {
		a->SetActorValue(*Globals::vestTier, 0);
	}
}

void Utils::CalculateArmorTiers(RE::Actor* a)
{
	uint16_t vestAR = 0;
	uint16_t helmetAR = 0;
	if (!a->inventoryList) {
		return;
	}
	//개조 정보도 불러오기 위해서 현재 인벤토리에 있는 아이템 중 장착된 방어구를 전부 훑고 지나가면서 인스턴스 데이터를 통해 방어력 수치를 가져옴
	for (auto invitem = a->inventoryList->data.begin(); invitem != a->inventoryList->data.end(); ++invitem) {
		if (invitem->object->formType == RE::ENUM_FORM_ID::kARMO) {
			RE::TESObjectARMO* invarmor = static_cast<RE::TESObjectARMO*>(invitem->object);
			RE::TESObjectARMO::InstanceData* invdata = &(invarmor->armorData);
			RE::TESObjectARMO::InstanceData* instanceData = invdata;
			if (invitem->stackData->IsEquipped() && instanceData != NULL) {
				if (invarmor->bipedModelData.bipedObjectSlots & 0x800 || invarmor->bipedModelData.bipedObjectSlots & 0x40) {  //Vest
					RE::ExtraInstanceData* extraInstanceData = (RE::ExtraInstanceData*)invitem->stackData->extra->GetByType(RE::EXTRA_DATA_TYPE::kInstanceData);
					uint16_t currentVestAR = 0;
					if (extraInstanceData) {
						instanceData = ((RE::TESObjectARMO::InstanceData*)extraInstanceData->data.get());
						currentVestAR = instanceData->rating;
					} else {
						currentVestAR = instanceData->rating;
					}

					if (invarmor->bipedModelData.bipedObjectSlots & 0x7) {  //Vest has helmet
						helmetAR = currentVestAR / 4;
					}
					vestAR += currentVestAR;
					if (instanceData->damageTypes) {
						for (auto it = instanceData->damageTypes->begin(); it != instanceData->damageTypes->end(); ++it) {
							if (it->first == Globals::dtPhysical) {
								vestAR += it->second.i + instanceData->rating;
							}
						}
					}
				} else if (invarmor->bipedModelData.bipedObjectSlots & 0x1) {  //Helmet exclusive
					RE::ExtraInstanceData* extraInstanceData = (RE::ExtraInstanceData*)invitem->stackData->extra->GetByType(RE::EXTRA_DATA_TYPE::kInstanceData);
					if (extraInstanceData) {
						instanceData = ((RE::TESObjectARMO::InstanceData*)extraInstanceData->data.get());
						helmetAR = instanceData->rating;
					} else {
						helmetAR = invdata->rating;
					}
					if (instanceData->damageTypes) {
						for (auto it = instanceData->damageTypes->begin(); it != instanceData->damageTypes->end(); ++it) {
							if (it->first == Globals::dtPhysical) {
								helmetAR += it->second.i + instanceData->rating;
							}
						}
					}
				}
			}
		}
	}
	SetVestTier(a, vestAR);
	SetHelmetTier(a, helmetAR);
}

std::string Utils::SplitString(const std::string str, const std::string delimiter, std::string& remainder)
{
	std::string ret;
	size_t i = str.find(delimiter);
	if (i == std::string::npos) {
		ret = str;
		remainder = "";
		return ret;
	}

	ret = str.substr(0, i);
	remainder = str.substr(i + 1);
	return ret;
}

float Utils::BSRandomFloat(float f1, float f2)
{
	typedef float (*FnBSRandomFloat)(float, float);
	REL::Relocation<FnBSRandomFloat> func{ REL::ID(1118937) };
	return func(f1, f2);
}

WCHAR* Utils::GetClipboard()
{
	WCHAR* strData{};

	if (OpenClipboard(NULL)) {
		HANDLE hClipboardData = GetClipboardData(CF_UNICODETEXT);
		if (hClipboardData) {
			WCHAR* pchData = (WCHAR*)GlobalLock(hClipboardData);
			if (pchData) {
				strData = pchData;
				GlobalUnlock(hClipboardData);
			}
		}
		CloseClipboard();
	}
	return strData;
}

uint32_t Translation::ReadLine_w(RE::BSResourceNiBinaryStream& stream, wchar_t* dst, uint32_t dstLen, uint32_t terminator)
{
	wchar_t* iter = dst;

	if (dstLen == 0)
		return 0;

	for (uint32_t i = 0; i < dstLen - 1; i++) {
		wchar_t data;

		if (stream.binary_read(&data, sizeof(data)) != sizeof(data))
			break;

		if (data == terminator)
			break;

		*iter++ = data;
	}

	// null terminate
	*iter = 0;

	return iter - dst;
}

void Translation::ParseTranslation(RE::BSScaleformTranslator* translator, std::string name)
{
	RE::Setting* setting = RE::INISettingCollection::GetSingleton()->GetSetting("sLanguage:General");
	if (!setting)
		setting = RE::INIPrefSettingCollection::GetSingleton()->GetSetting("sLanguage:General");
	std::string path = "Interface\\Translations\\";

	// Construct translation filename
	path += name;
	path += "_";
	path += (setting && setting->GetType() == RE::Setting::SETTING_TYPE::kString) ? setting->GetString() : "en";
	path += ".txt";

	if (!std::filesystem::exists(path))
		path = "Interface\\Translations\\" + name + "_en.txt";

	RE::BSResourceNiBinaryStream fileStream(path.c_str());

	// Check if file is empty, if not check if the BOM is UTF-16
	uint16_t bom = 0;
	uint32_t ret = (uint32_t)fileStream.binary_read(&bom, sizeof(uint16_t));
	if (ret == 0) {
		logger::error("Empty translation file.");
		return;
	}
	if (bom != 0xFEFF) {
		logger::error("BOM Error, file must be encoded in UCS-2 LE.");
		return;
	}

	while (true) {
		wchar_t buf[512];
		uint32_t len = ReadLine_w(fileStream, buf, sizeof(buf) / sizeof(buf[0]), '\n');
		if (len == 0)  // End of file
			return;

		// at least $ + wchar_t + \t + wchar_t
		if (len < 4 || buf[0] != '$')
			continue;

		wchar_t last = buf[len - 1];
		if (last == '\r')
			len--;

		// null terminate
		buf[len] = 0;

		uint32_t delimIdx = 0;
		for (uint32_t i = 0; i < len; i++)
			if (buf[i] == '\t')
				delimIdx = i;

		// at least $ + wchar_t
		if (delimIdx < 2)
			continue;

		// replace \t by \0
		buf[delimIdx] = 0;

		RE::BSFixedStringWCS key(buf);
		RE::BSFixedStringWCS translation(&buf[delimIdx + 1]);

		RE::BSTTuple<RE::BSFixedStringWCS, RE::BSFixedStringWCS> item(key, translation);
		translator->translator.translationMap.insert(item);
	}

	logger::info("Translation for {} injected.", name);
}

bool Utils::PlaySound(RE::BGSSoundDescriptorForm* sndr, RE::NiPoint3 pos, RE::NiAVObject* node)
{
	typedef bool* func_t(Unk, RE::BGSSoundDescriptorForm*, RE::NiPoint3, RE::NiAVObject*);
	REL::Relocation<func_t> func{ REL::ID(376497) };
	Unk u;
	return func(u, sndr, pos, node);
}

void Utils::ShakeCamera(float mul, RE::NiPoint3 origin, float duration, float strength)
{
	using func_t = decltype(&Utils::ShakeCamera);
	REL::Relocation<func_t> func{ REL::ID(758209) };
	return func(mul, origin, duration, strength);
}

void Utils::ApplyImageSpaceModifier(RE::TESImageSpaceModifier* imod, float strength, RE::NiAVObject* target)
{
	using func_t = decltype(&Utils::ApplyImageSpaceModifier);
	REL::Relocation<func_t> func{ REL::ID(179769) };
	return func(imod, strength, target);
}
