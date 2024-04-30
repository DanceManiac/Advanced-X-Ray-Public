#ifndef	dxUIShader_included
#define	dxUIShader_included
#pragma once

#include "..\..\Include\xrRender\UIShader.h"

class dxUIShader : public IUIShader
{
	friend class dxUIRender;
	friend class dxDebugRender;
	friend class dxWallMarkArray;
	friend class CRender;
public:
	virtual		~dxUIShader(){;}
	virtual void Copy(IUIShader &_in);
	virtual void create(LPCSTR sh, LPCSTR tex=0);
	virtual bool inited() {return hShader;}
	virtual void destroy();
	virtual LPCSTR GetTexName() { return dbg_tex; }
	virtual LPCSTR GetShaderName() { return dbg_shader; }
private:
	ref_shader		hShader;
	LPCSTR			dbg_shader{};
	LPCSTR			dbg_tex{};
};

#endif	//	dxUIShader_included