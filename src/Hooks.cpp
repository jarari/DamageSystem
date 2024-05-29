#include <Hooks.h>
#include <MenuWatcher.h>
#include <Globals.h>
#include <PlayerDeathWatcher.h>
#include <HitEventWatcher.h>
#include <ObjectLoadWatcher.h>
#include <EquipWatcher.h>

void Hooks::InitializeHooks()
{
	RE::UI::GetSingleton()->GetEventSource<RE::MenuOpenCloseEvent>()->RegisterSink(MenuWatcher::GetSingleton());
	RE::EquipEventSource::GetSingleton()->RegisterSink(EquipWatcher::GetSingleton());
	RE::ObjectLoadedEventSource::GetSingleton()->RegisterSink(ObjectLoadWatcher::GetSingleton());
	RE::HitEventSource::GetSingleton()->RegisterSink(HitEventWatcher::GetSingleton());
	((RE::BSTEventSource<RE::BGSActorDeathEvent>*)Globals::p)->RegisterSink(PlayerDeathWatcher::GetSingleton());
}
