#pragma once

class CBlender_rain_drops : public IBlender
{
public:
	virtual		LPCSTR		getComment() { return "Rain drops"; }
	virtual		BOOL		canBeDetailed()	{ return FALSE;	}
	virtual		BOOL		canBeLMAPped()	{ return FALSE;	}

	virtual		void		Compile			(CBlender_Compile& C);

	CBlender_rain_drops();
	virtual ~CBlender_rain_drops();
};