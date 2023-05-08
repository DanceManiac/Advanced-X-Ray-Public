// hit_immunity.h: ����� ��� ��� ��������, ������� ������������
//				   ������������ ���������� ��� ������ ����� �����
//////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_space.h"
#include "hit_immunity_space.h"

class CHitImmunity
{
	//������������ �� ������� ����������� ���
	//��� ��������������� ���� �����������
	//(��� �������� �������� � ����������� ��������)
private:
	HitImmunity::HitTypeSVec m_HitImmunityKoefs;
public:
						CHitImmunity	();
	virtual				~CHitImmunity	();

			void		LoadImmunities	(LPCSTR section, CInifile const * ini);
			void		AddImmunities	(LPCSTR section, CInifile const * ini);
			void SetHitImmunity(ALife::EHitType hit_type, float val) { m_HitImmunityKoefs[hit_type] = val; }
			float		GetHitImmunity	(ALife::EHitType hit_type) {return m_HitImmunityKoefs[hit_type];}
			float		AffectHit		(float power, ALife::EHitType hit_type) {return power*GetHitImmunity(hit_type);}
};