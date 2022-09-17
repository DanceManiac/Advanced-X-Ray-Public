// hit_immunity.h: ����� ��� ��� ��������, ������� ������������
//				   ������������ ���������� ��� ������ ����� �����
//////////////////////////////////////////////////////////////////////

#pragma once

#include "alife_space.h"
#include "hit_immunity_space.h"

class CHitImmunity
{
protected:
	//������������ �� ������� ����������� ���
	//��� ��������������� ���� �����������
	//(��� �������� �������� � ����������� ��������)
	HitImmunity::HitTypeSVec m_HitTypeK;
public:
	CHitImmunity();
	virtual ~CHitImmunity();

	virtual void LoadImmunities(LPCSTR section, CInifile const * ini);
	float		GetHitImmunity	(ALife::EHitType hit_type) { return m_HitTypeK[hit_type]; }
	virtual float AffectHit		(float power, ALife::EHitType hit_type);
};