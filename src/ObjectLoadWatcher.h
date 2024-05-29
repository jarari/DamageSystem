#pragma once

class ObjectLoadWatcher : public RE::BSTEventSink<RE::TESObjectLoadedEvent>
{
protected:
	static ObjectLoadWatcher* instance;

public:
	ObjectLoadWatcher() = default;
	ObjectLoadWatcher(ObjectLoadWatcher&) = delete;
	void operator=(const ObjectLoadWatcher&) = delete;
	static ObjectLoadWatcher* GetSingleton()
	{
		if (!instance)
			instance = new ObjectLoadWatcher();
		return instance;
	}
	virtual RE::BSEventNotifyControl ProcessEvent(const RE::TESObjectLoadedEvent& evn, RE::BSTEventSource<RE::TESObjectLoadedEvent>* src) override;
};
