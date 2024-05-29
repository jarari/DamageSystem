#pragma once

class HitEventWatcher : public RE::BSTEventSink<RE::TESHitEvent> {
protected:
	static HitEventWatcher* instance;

public:
	HitEventWatcher() = default;
	HitEventWatcher(HitEventWatcher&) = delete;
	void operator=(const HitEventWatcher&) = delete;
	static HitEventWatcher* GetSingleton()
	{
		if (!instance)
			instance = new HitEventWatcher();
		return instance;
	}
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESHitEvent& evn, RE::BSTEventSource<RE::TESHitEvent>* src) override;
};
