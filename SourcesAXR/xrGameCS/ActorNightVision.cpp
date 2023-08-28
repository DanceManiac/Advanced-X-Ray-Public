#include "stdafx.h"

#include "Actor.h"
#include "level.h"
#include "actorEffector.h"
#include "ai_sounds.h"

#include "ActorNightVision.h"

ENGINE_API extern int ps_r__ShaderNVG;

CNightVisionEffector::CNightVisionEffector(const shared_str& section)
{
	m_sounds.LoadSound(section.c_str(), "snd_night_vision_on", "NightVisionOnSnd", false, SOUND_TYPE_ITEM_USING);
	m_sounds.LoadSound(section.c_str(), "snd_night_vision_off", "NightVisionOffSnd", false, SOUND_TYPE_ITEM_USING);
	m_sounds.LoadSound(section.c_str(), "snd_night_vision_idle", "NightVisionIdleSnd", true, SOUND_TYPE_ITEM_USING);
	m_sounds.LoadSound(section.c_str(), "snd_night_vision_broken", "NightVisionBrokenSnd", false, SOUND_TYPE_ITEM_USING);
}

void CNightVisionEffector::Start(const shared_str& sect, CActor* pA, bool play_sound)
{
	if (ps_r__ShaderNVG == 0)
		AddEffector(pA, effNightvision, sect);

	if (play_sound)
	{
		PlaySounds(eStartSound);
		PlaySounds(eIdleSound);
	}
}

void CNightVisionEffector::Stop(const float factor, bool play_sound)
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
	if (!pActor)		return;
	CEffectorPP* pp = pActor->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
	if (pp)
	{
		pp->Stop(factor);
	}

	if (play_sound)
	{
		m_sounds.StopSound("NightVisionIdleSnd");

		if (ps_r__ShaderNVG == 0) //Временная затычка
			PlaySounds(eStopSound);
	}
}

void CNightVisionEffector::StartForScope(const shared_str& sect, CActor* pA, bool play_sound)
{
	AddEffector(pA, effNightvision, sect);

	if (play_sound)
	{
		PlaySounds(eStartSound);
		PlaySounds(eIdleSound);
	}
}

void CNightVisionEffector::StopForScope(const float factor, bool play_sound)
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
	if (!pActor)		return;
	CEffectorPP* pp = pActor->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
	if (pp)
	{
		pp->Stop(factor);
		if (play_sound)
			PlaySounds(eStopSound);
	}
}

void CNightVisionEffector::StopOnlyEffector(const float factor)
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
	if (!pActor)		return;
	CEffectorPP* pp = pActor->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
	if (pp)
		pp->Stop(factor);
}

bool CNightVisionEffector::IsActive()
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
	if (!pActor)		return false;
	CEffectorPP* pp = pActor->Cameras().GetPPEffector((EEffectorPPType)effNightvision);
	return (pp != NULL);
}

void CNightVisionEffector::OnDisabled(CActor* pA, bool play_sound)
{
	if (play_sound)
		PlaySounds(eBrokeSound);
}

void CNightVisionEffector::PlaySounds(EPlaySounds which)
{
	CActor* pActor = smart_cast<CActor*>(Level().CurrentControlEntity());
	if (!pActor)
		return;

	bool bPlaySoundFirstPerson = !!pActor->HUDview();
	switch (which)
	{
	case eStartSound:
	{
		m_sounds.PlaySound("NightVisionOnSnd", pActor->Position(), NULL, bPlaySoundFirstPerson);
	}break;
	case eStopSound:
	{
		m_sounds.PlaySound("NightVisionOffSnd", pActor->Position(), NULL, bPlaySoundFirstPerson);
	}break;
	case eIdleSound:
	{
		m_sounds.PlaySound("NightVisionIdleSnd", pActor->Position(), NULL, bPlaySoundFirstPerson, true);
	}break;
	case eBrokeSound:
	{
		m_sounds.PlaySound("NightVisionBrokenSnd", pActor->Position(), NULL, bPlaySoundFirstPerson);
	}break;
	default: NODEFAULT;
	}
}