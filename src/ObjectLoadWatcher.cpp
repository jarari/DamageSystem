#include <Configs.h>
#include <ObjectLoadWatcher.h>
#include <Utils.h>

ObjectLoadWatcher* ObjectLoadWatcher::instance = nullptr;
RE::BSEventNotifyControl ObjectLoadWatcher::ProcessEvent(const RE::TESObjectLoadedEvent& evn, [[maybe_unused]] RE::BSTEventSource<RE::TESObjectLoadedEvent>* src)
{
	if (!evn.loaded) {
		return RE::BSEventNotifyControl::kContinue;
	}
	RE::TESForm* form = RE::TESForm::GetFormByID(evn.formId);
	if (form && form->formType == RE::ENUM_FORM_ID::kACHR) {
		RE::Actor* a = static_cast<RE::Actor*>(form);
		Utils::CalculateArmorTiers(a);
	}
	return RE::BSEventNotifyControl::kContinue;
}
