#include <Configs.h>
#include <EquipWatcher.h>
#include <Utils.h>

EquipWatcher* EquipWatcher::instance = nullptr;
RE::BSEventNotifyControl EquipWatcher::ProcessEvent(const RE::TESEquipEvent& evn, [[maybe_unused]] RE::BSTEventSource<RE::TESEquipEvent>* src)
{
	RE::TESForm* item = RE::TESForm::GetFormByID(evn.formId);
	if (item && item->formType == RE::ENUM_FORM_ID::kARMO) {
		Utils::CalculateArmorTiers(evn.a);
	}
	return RE::BSEventNotifyControl::kContinue;
}
