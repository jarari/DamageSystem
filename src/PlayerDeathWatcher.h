#pragma once

class PlayerDeathWatcher : public RE::BSTEventSink<RE::BGSActorDeathEvent>
{
protected:
	static PlayerDeathWatcher* instance;

public:
	PlayerDeathWatcher() = default;
	PlayerDeathWatcher(PlayerDeathWatcher&) = delete;
	void operator=(const PlayerDeathWatcher&) = delete;
	static PlayerDeathWatcher* GetSingleton()
	{
		if (!instance)
			instance = new PlayerDeathWatcher();
		return instance;
	}
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::BGSActorDeathEvent& evn, RE::BSTEventSource<RE::BGSActorDeathEvent>* src) override;
};
