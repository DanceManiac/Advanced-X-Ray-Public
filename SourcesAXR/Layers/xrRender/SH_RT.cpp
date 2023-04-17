#include "stdafx.h"
#pragma hdrstop

#include "ResourceManager.h"

#include "dxRenderDeviceRender.h"

CRT::CRT			()
{
	pSurface		= NULL;
	pRT				= NULL;
	dwWidth			= 0;
	dwHeight		= 0;
	fmt				= D3DFMT_UNKNOWN;
}
CRT::~CRT			()
{
	destroy			();

	// release external reference
	DEV->_DeleteRT	(this);
}

void CRT::create	(LPCSTR Name, u32 w, u32 h,	D3DFORMAT f, u32 SampleCount )
{
	if (pSurface)	return;

	R_ASSERT	(HW.pDevice && Name && Name[0] && w && h);
	_order		= CPU::GetCLK()	;	//RDEVICE.GetTimerGlobal()->GetElapsed_clk();

	HRESULT		_hr;

	dwWidth		= w;
	dwHeight	= h;
	fmt			= f;

	// Get caps
	D3DCAPS9	caps;
	R_CHK		(HW.pDevice->GetDeviceCaps(&caps));

	// Pow2
	if (!btwIsPow2(w) || !btwIsPow2(h))
	{
		if (!HW.Caps.raster.bNonPow2)	return;
	}

	// Check width-and-height of render target surface
	if (w>caps.MaxTextureWidth)			return;
	if (h>caps.MaxTextureHeight)		return;

	// Select usage
	u32 usage	= 0;
	if (D3DFMT_D24X8==fmt)									usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D24S8		==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D15S1		==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16			==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16_LOCKABLE==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if ((D3DFORMAT)MAKEFOURCC('D','F','2','4') == fmt)	usage = D3DUSAGE_DEPTHSTENCIL;
	else													usage = D3DUSAGE_RENDERTARGET;

	// Validate render-target usage
	_hr = HW.pD3D->CheckDeviceFormat(
		HW.DevAdapter,
		HW.DevT,
		HW.Caps.fTarget,
		usage,
		D3DRTYPE_TEXTURE,
		f
		);
	if (FAILED(_hr))					return;

	// Try to create texture/surface
	DEV->Evict				();
	_hr = HW.pDevice->CreateTexture		(w, h, 1, usage, f, D3DPOOL_DEFAULT, &pSurface,NULL);
	HW.stats_manager.increment_stats_rtarget	( pSurface );

	if (FAILED(_hr) || (0==pSurface))	return;

	// OK
#ifdef DEBUG
	Msg			("* created RT(%s), %dx%d",Name,w,h);
#endif // DEBUG
	R_CHK		(pSurface->GetSurfaceLevel	(0,&pRT));
	pTexture	= DEV->_CreateTexture	(Name);
	pTexture->surface_set	(pSurface);
}

void CRT::destroy		()
{
	if (pTexture._get())
	{
		pTexture->surface_set	(0);
		pTexture.destroy();
		pTexture = nullptr;
	}
	
	_RELEASE	(pRT		);

	HW.stats_manager.decrement_stats_rtarget	( pSurface );
	_RELEASE	(pSurface	);
}
void CRT::reset_begin	()
{
	destroy		();
}
void CRT::reset_end		()
{
	create		(*cName,dwWidth,dwHeight,fmt);
}
void resptrcode_crt::create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount )
{
	_set			(DEV->_CreateRT(Name,w,h,f));
}