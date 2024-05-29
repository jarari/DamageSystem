#include "Configs.h"
#include "PlayerDeathWatcher.h"
#include <Globals.h>

PlayerDeathWatcher* PlayerDeathWatcher::instance = nullptr;
RE::BSEventNotifyControl PlayerDeathWatcher::ProcessEvent(const RE::BGSActorDeathEvent& evn, [[maybe_unused]] RE::BSTEventSource<RE::BGSActorDeathEvent>* src)
{
	logger::info("---Player Death---");
	RE::ActiveEffectList* aeList = Globals::p->GetActiveEffectList();
	if (aeList) {
		for (auto it = aeList->data.begin(); it != aeList->data.end(); ++it) {
			RE::ActiveEffect* ae = it->get();
			if (ae && !(ae->flags & RE::ActiveEffect::Flags::kInactive)) {
				RE::EffectSetting* avEffectSetting = *(RE::EffectSetting**)((uint64_t)(it->get()->effect) + 0x10);
				if (avEffectSetting && avEffectSetting->data.primaryAV == Globals::health) {
					logger::info("Active Effect : {} with magnitude {}", avEffectSetting->fullName.c_str(), ae->magnitude);
				}
			}
		}
	};
	logger::info("Head Condition\t{}", Globals::p->GetActorValue(*Globals::perceptionCondition));
	logger::info("Torso Condition\t{}", Globals::p->GetActorValue(*Globals::enduranceCondition));
	logger::info("LArm Condition\t{}", Globals::p->GetActorValue(*Globals::leftAttackCondition));
	logger::info("LLeg Condition\t{}", Globals::p->GetActorValue(*Globals::leftMobilityCondition));
	logger::info("RArm Condition\t{}", Globals::p->GetActorValue(*Globals::rightAttackCondition));
	logger::info("RLeg Condition\t{}", Globals::p->GetActorValue(*Globals::rightMobilityCondition));
	logger::info("Last Health remaining\t{}", evn.lastHealth);
	logger::info("Last Damage taken\t{}", evn.damageTaken);
	return RE::BSEventNotifyControl::kContinue;
}
