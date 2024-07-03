#include <Configs.h>
#include <HitEventWatcher.h>
#include <Utils.h>
#include <Globals.h>

std::random_device rd;
HitEventWatcher* HitEventWatcher::instance = nullptr;
RE::BSEventNotifyControl HitEventWatcher::ProcessEvent(const RE::TESHitEvent& evn, [[maybe_unused]] RE::BSTEventSource<RE::TESHitEvent>* src)
{
	if (!evn.hasHitData) {
		return RE::BSEventNotifyControl::kContinue;
	}

	if (evn.hitdata.source.object) {
		RE::TESObjectWEAP::InstanceData* weapInstance = (RE::TESObjectWEAP::InstanceData*)evn.hitdata.source.instanceData.get();
		RE::TESObjectREFR* attacker = evn.hitdata.attackerHandle.get().get();
		RE::Actor* victim = static_cast<RE::Actor*>(evn.hitdata.victimHandle.get().get());
		RE::Projectile* proj = static_cast<RE::Projectile*>(evn.hitdata.sourceHandle.get().get());

		if (!victim || victim->formType != RE::ENUM_FORM_ID::kACHR) {
			return RE::BSEventNotifyControl::kContinue;
		}

		if (victim->IsDead(true)) {
			return RE::BSEventNotifyControl::kContinue;
		}

		if (evn.hitdata.impactData.collisionObj) {
			if (!proj || proj->data.objectReference->formType != RE::ENUM_FORM_ID::kPROJ) {
				return RE::BSEventNotifyControl::kContinue;
			}

			bool shouldLog = victim == Globals::p || attacker == Globals::p;

			RE::NiAVObject* parent = evn.hitdata.impactData.collisionObj->sceneObject;
			if (shouldLog) {
				logger::info("Projectile {:08X} (formid {:08X})", (uintptr_t)proj, proj->formID);
			}

			bool targetUnknown = false;
			RE::TESNPC* victimNPC = victim->GetNPC();
			if (victimNPC) {
				if (!victimNPC->fullName.c_str() || victimNPC->fullName.length() == 0) {
					targetUnknown = true;
				}
			} else {
				targetUnknown = true;
			}

			if (!targetUnknown) {
				if (victim == Globals::p) {
					if (shouldLog) {
						logger::info("{}(Player) got hit on {}", victimNPC->fullName.c_str(), parent->name.c_str());
					}
				} else {
					if (shouldLog) {
						logger::info("{}({}) got hit on {}", victimNPC->fullName.c_str(), (uintptr_t)victim, parent->name.c_str());
					}
				}
			} else {
				if (shouldLog) {
					logger::info("Unknown({}) got hit on %s", (uintptr_t)victim, parent->name.c_str());
				}
			}

			int partType = evn.hitdata.bodypartType;
			if (partType == -1) {
				//손은 파트 타입이 지정되지 않아있어서 하드코딩
				if (parent->name == "LArm_Hand") {
					partType = 6;
				} else if (parent->name == "RArm_Hand") {
					partType = 8;
				} else {
					partType = 0;
				}
			}

			//로그에 탄약 타입 표기
			RE::TESAmmo* ammo = evn.hitdata.ammo;
			if (!ammo && weapInstance) {
				ammo = weapInstance->ammo;
			}
			if (ammo) {
				if (shouldLog) {
					logger::info("Ammo : {} (FormID {:08X}) - Projectile FormID {:08X}",
						ammo->fullName.c_str(), ammo->formID,
						ammo->data.projectile->formID);
				}
			}

			//데스마크 제외 종족 확인
			int i = 0;
			while (i < Globals::deathMarkExcludedList.size()) {
				if (Globals::deathMarkExcludedList[i] == victim->race) {
					return RE::BSEventNotifyControl::kContinue;
				}
				++i;
			}

			//AVIF에 마지막으로 맞은 부위 저장
			int partFound = 0;
			i = 0;
			std::vector<RE::BGSBodyPartData::PartType> partsList[] = { Globals::headParts, Globals::torsoParts, Globals::larmParts, Globals::llegParts, Globals::rarmParts, Globals::rlegParts };
			while (partFound == 0 && i < 6) {
				for (auto part = partsList[i].begin(); part != partsList[i].end(); ++part) {
					if (partType == *part) {
						victim->SetActorValue(*Globals::lasthitpart, i + 1);
						partFound = i + 1;
					}
				}
				++i;
			}
			if (!partFound) {
				victim->SetActorValue(*Globals::lasthitpart, 7);
			} else {
				/*
				* 0x10000	MissileProjectile
				* 0x20000	GrenadeProjectile
				* 0x40000	BeamProjectile
				* 0x80000	FlameProjectile
				* 0x100000	ConeProjectile
				* 0x200000	BarrierProjectile
				* 0x400000	ArrowProjectile
				*/
				uint32_t projType = (static_cast<RE::BGSProjectile*>(proj->data.objectReference)->data.flags & 0x7F0000);
				bool hasDeathChance = false;
				uint16_t chance = 0;
				if (projType == 0x40000 || projType == 0x80000 || projType == 0x200000) {  //에너지
					if (shouldLog) {
						logger::info("Laser projectile with damage {}", evn.hitdata.calculatedBaseDamage);
					}
					CartridgeData& gcd = Configs::cartridgeData.at("Laser");
					if (partFound <= 2) {
						if (partFound == 1) {  //머리
							float armorTier = victim->GetActorValue(*Globals::helmetTier);
							float deathCondition = min(gcd.deathConditions[0] - Configs::headFatalityDecPerTier * armorTier, 100.f);
							if (shouldLog) {
								logger::info("Helmet Tier {}", armorTier);
								logger::info("Head Death Condition: {}, Current Head Condition: {}", deathCondition, victim->GetActorValue(*Globals::perceptionCondition));
							}
							if (deathCondition >= victim->GetActorValue(*Globals::perceptionCondition) || deathCondition >= victim->GetActorValue(*Globals::brainCondition) || armorTier == 0) {
								chance = victim->GetActorValue(*Globals::perceptionCondition) == 0 ? 0 : gcd.protectionChances[(int)armorTier];
								hasDeathChance = true;
							}
						} else if (partFound == 2) {  //몸통
							float armorTier = victim->GetActorValue(*Globals::vestTier);
							float deathCondition = min(gcd.deathConditions[1] - Configs::torsoFatalityDecPerTier * armorTier, 100.f);
							if (shouldLog) {
								logger::info("Vest Tier {}", armorTier);
								logger::info("Torso Death Condition: {}, Current Torso Condition: {}", deathCondition, victim->GetActorValue(*Globals::enduranceCondition));
							}
							if (deathCondition >= victim->GetActorValue(*Globals::enduranceCondition)) {
								chance = victim->GetActorValue(*Globals::enduranceCondition) == 0 ? 0 : gcd.protectionChances[(int)armorTier];
								hasDeathChance = true;
							}
						}
					}
				} else {
					if (shouldLog) {
						logger::info("Physical projectile with damage {}", evn.hitdata.calculatedBaseDamage);
					}
					RE::ActiveEffectList* aeList = victim->GetActiveEffectList();  //마법 효과 리스트를 가져오는 부분
					if (aeList) {
						BleedingData& bld = Configs::bleedingConfigs.at((EFDBodyParts)partFound);
						if (shouldLog) {
							logger::info("Bleeding Condition {}, Current Condition {}", bld.conditionStart, victim->GetActorValue(**Globals::EFDConditions[partFound]));
						}

						// 일반출혈, 골절 적용 파트
						if (bld.conditionStart >= victim->GetActorValue(**Globals::EFDConditions[partFound])) {  // 내구도 확인

							//일반출혈 파트
							float bleedChance = bld.chanceMin + (bld.chanceMax - bld.chanceMin) * 
								(bld.conditionStart - max(bld.conditionThreshold, victim->GetActorValue(**Globals::EFDConditions[partFound]))) / (bld.conditionStart - bld.conditionThreshold);
							std::mt19937 e{ rd() };
							std::uniform_real_distribution<float> dist{ 0, 100 };
							float result = dist(e);
							if (shouldLog) {
								logger::info("Bleeding Chance {}, Bleeding Avoid Chance {}", bleedChance, result);
							}
							if (result && result <= bleedChance) {
								//마법 효과 리스트에서 출혈 효과를 찾아보고 출혈 효과가 있으면 강도 증가 + 갱신, 없으면 새로운 출혈 효과 적용
								RE::ActiveEffect* bleedae = nullptr;
								for (auto it = aeList->data.begin(); it != aeList->data.end(); ++it) {
									RE::ActiveEffect* ae = it->get();
									if (ae && !(ae->flags & RE::ActiveEffect::Flags::kInactive) && ae->item == bld.spell) {
										bleedae = ae;
										if (shouldLog) {
											logger::info("{} is already bleeding", Globals::EFDBodyPartsName[partFound].c_str());
										}
										break;  //break 걸면 최상단에 있는 출혈만 효과가 강해짐
									}
								}
								float bleedmag = evn.hitdata.calculatedBaseDamage * bld.multiplier;
								if (bleedae) {
									bleedae->magnitude -= bleedmag;
									bleedae->elapsed = 0;
									if (shouldLog) {
										logger::info("Current bleeding magnitude {}", bleedae->magnitude * -1.0f);
									}
								} else {
									bld.spell->listOfEffects[0]->data.magnitude = bld.initialDamage + bleedmag;
									bld.spell->listOfEffects[1]->data.magnitude = bld.initialDamage + bleedmag;
									bld.spell->Cast(victim, victim, victim, RE::GameVM::GetSingleton()->GetVM().get());
									if (shouldLog) {
										logger::info("Bleeding start magnitude {}", bld.initialDamage + bleedmag);
									}
								}
							}

							//골절 파트
							if (partFound >= 3) {
								std::mt19937 e4{ rd() };
								std::uniform_real_distribution<float> dist4{ 0, 100 };
								float fractureResult = dist4(e4);
								if (shouldLog) {
									logger::info("Fracture Chance {}, Fracture Avoid Chace {}", bld.fractureChance, fractureResult);
								}

								if (victim->GetActorValue(**Globals::EFDConditions[partFound]) == 0) {
									fractureResult = 0;
								}

								if (fractureResult <= bld.fractureChance) {
									RE::ActiveEffect* fractureae = nullptr;
									for (auto it = aeList->data.begin(); it != aeList->data.end(); ++it) {
										RE::ActiveEffect* ae = it->get();
										if (ae && !(ae->flags & RE::ActiveEffect::Flags::kInactive) && ae->item == bld.fractureSpell) {
											fractureae = ae;
											if (shouldLog) {
												logger::info("{} is already Fractured", Globals::EFDBodyPartsName[partFound].c_str());
											}
										}
									}
									if (!fractureae) {
										if (shouldLog) {
											logger::info("{} Fracture Apply", Globals::EFDBodyPartsName[partFound].c_str());
										}
										bld.fractureSpell->Cast(victim, victim, victim, RE::GameVM::GetSingleton()->GetVM().get());
									}
								}
							}
						}

					} else {
						if (shouldLog) {
							logger::info("ActiveEffectList not found. Bleeding can't be processed");
						}
					}

					CartridgeData& gcd = Configs::cartridgeData.at("Global");
					if (partFound <= 2) {
						if (partFound == 1) {  //머리
							float armorTier = victim->GetActorValue(*Globals::helmetTier);
							float fatalityIncrement = pow(evn.hitdata.calculatedBaseDamage / Configs::fi.formulaA, Configs::fi.formulaB) * Configs::fi.formulaC * Configs::headFatalityDecPerTier;
							float deathCondition = min(gcd.deathConditions[0] - Configs::headFatalityDecPerTier * armorTier + fatalityIncrement, 100.f);
							if (shouldLog) {
								logger::info("Helmet Tier {}", armorTier);
								logger::info("Head Death Condition: {}, Current Head Condition: {}", deathCondition, victim->GetActorValue(*Globals::perceptionCondition));
							}
							if (deathCondition >= victim->GetActorValue(*Globals::perceptionCondition) || deathCondition >= victim->GetActorValue(*Globals::brainCondition) || armorTier == 0) {
								float protectionChance = (uint16_t)max(min(log10(Configs::helmetRatings[armorTier] / Configs::gdHead.formulaA + 1.0f) * Configs::gdHead.formulaB + (Configs::gdHead.formulaC - evn.hitdata.calculatedBaseDamage) * 0.25f + Configs::gdHead.formulaD, 100), 0);
								chance = victim->GetActorValue(*Globals::perceptionCondition) == 0 ? 0 : protectionChance;
								hasDeathChance = true;
							}
						} else if (partFound == 2) {  //몸통
							float armorTier = victim->GetActorValue(*Globals::vestTier);
							float fatalityIncrement = pow(evn.hitdata.calculatedBaseDamage / Configs::fi.formulaA, Configs::fi.formulaB) * Configs::fi.formulaC * Configs::torsoFatalityDecPerTier;
							float deathCondition = min(gcd.deathConditions[1] - Configs::torsoFatalityDecPerTier * armorTier + fatalityIncrement, 100.f);
							if (shouldLog) {
								logger::info("Vest Tier {}", armorTier);
								logger::info("Torso Death Condition: {}, Current Torso Condition: {}", deathCondition, victim->GetActorValue(*Globals::enduranceCondition));
							}
							if (deathCondition >= victim->GetActorValue(*Globals::enduranceCondition)) {
								float protectionChance = (uint16_t)max(min(log10(Configs::vestRatings[armorTier] / Configs::gdTorso.formulaA + 1.0f) * Configs::gdTorso.formulaB + (Configs::gdTorso.formulaC - evn.hitdata.calculatedBaseDamage) * 0.25f + Configs::gdTorso.formulaD, 100), 0);
								chance = victim->GetActorValue(*Globals::enduranceCondition) == 0 ? 0 : protectionChance;
								hasDeathChance = true;
							}
						}
					}
				}

				if (hasDeathChance) {
					std::mt19937 e{ rd() };  // or std::default_random_engine e{rd()};
					std::uniform_int_distribution<uint16_t> dist{ 1, 100 };
					uint16_t result = dist(e);
					if (shouldLog) {
						logger::info("Death Avoid Chance {} vs Death Chance {}", chance, result);
					}
					if (result > chance) {
						Globals::killDeathMarked->Cast(victim, victim, victim, RE::GameVM::GetSingleton()->GetVM().get());
						if (*Utils::ptr_engineTime - Globals::lastDeathMarkSoundTime > 0.01f && victim->Get3D()) {
							if (partFound == 1) {
								Utils::PlaySound(Globals::deathMarkHeadSound, victim->data.location, victim->Get3D());
							} else {
								Utils::PlaySound(Globals::deathMarkTorsoSound, victim->data.location, victim->Get3D());
							}
							Globals::lastDeathMarkSoundTime = *Utils::ptr_engineTime;
						}
						if (shouldLog) {
							logger::info("---DeathMark activated---");
						}
					}
					//즉사 회피시 메세지 표기
					else if (victim == Globals::p) {
						RE::SendHUDMessage::ShowHUDMessage("$DAMAGESYSTEM_DEATHAVOID", nullptr, true, true);
						Utils::ShakeCamera(1.0f, victim->data.location, 0.2f, 1.0f);
						Utils::PlaySound(Globals::avoidedDeathSound, victim->data.location, RE::PlayerCamera::GetSingleton()->cameraRoot.get());
						Utils::ApplyImageSpaceModifier(Globals::avoidedDeathIMOD, 1.0f, nullptr);
						if (*Utils::ptr_engineTime - Globals::lastAvoidedDeathBuzzingTime > 4.0f) {
							Utils::PlaySound(Globals::avoidedDeathBuzzing, victim->data.location, RE::PlayerCamera::GetSingleton()->cameraRoot.get());
							Globals::lastAvoidedDeathBuzzingTime = *Utils::ptr_engineTime;
						}
					}
				}
			}
		} else {
			if (evn.hitdata.attackData) {
				if (attacker && attacker->formType == RE::ENUM_FORM_ID::kACHR) {
				}
			}
		}
	}
	return RE::BSEventNotifyControl::kContinue;
}
