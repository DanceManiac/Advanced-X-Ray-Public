#pragma once


// ������ ���� ������ ��������
DEF_VECTOR(TEAM_SKINS_NAMES, xr_string);	

// ������ ���� ������ ��������
DEF_VECTOR(DEF_ITEMS_LIST, u16);	

//��������� ������ �� �������
struct		TeamStruct
{
	shared_str			caSection;		// ��� ������ ��������
	TEAM_SKINS_NAMES	aSkins;			// ������ ������ ��� �������
	DEF_ITEMS_LIST		aDefaultItems;	// ������ ��������� �� ���������

	//---- Money -------------------
	s32					m_iM_Start			;
	s32					m_iM_OnRespawn		;
	s32					m_iM_Min			;
	
	s32					m_iM_KillRival		;
	s32					m_iM_KillSelf		;
	s32					m_iM_KillTeam		;

	s32					m_iM_TargetRival	;
	s32					m_iM_TargetTeam		;
	s32					m_iM_TargetSucceed	;
	s32					m_iM_TargetSucceedAll	;
	s32					m_iM_TargetFailed	;

	s32					m_iM_RoundWin		;
	s32					m_iM_RoundLoose		;
	s32					m_iM_RoundDraw		;		

	s32					m_iM_RoundWin_Minor		;
	s32					m_iM_RoundLoose_Minor	;
	s32					m_iM_RivalsWipedOut		;
	//---------------------------------------------
	s32					m_iM_ClearRunBonus		;
	//---------------------------------------------
	float				m_fInvinsibleKillModifier;

};

//������ ������ �� ��������
DEF_DEQUE(TEAM_DATA_LIST, TeamStruct);



