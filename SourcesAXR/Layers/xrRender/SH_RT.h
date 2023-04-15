#ifndef SH_RT_H
#define SH_RT_H
#pragma once

#if defined (USE_DX11)
enum ViewPort;

struct RtCreationParams
{
	u32 w;
	u32 h;
	ViewPort viewport;

	RtCreationParams(u32 W, u32 H, ViewPort vp)
	{
		w = W;
		h = H;
		viewport = vp;
	};
};

struct ViewPortRT
{
	ViewPortRT()
	{
		rtWidth = 0;
		rtHeight = 0;

		textureSurface = nullptr;
		zBufferInstance = nullptr;
		renderTargetInstance = nullptr;
#ifdef USE_DX11
		unorderedAccessViewInstance = nullptr;
#endif
		shaderResView = nullptr;
	}

	ID3DTexture2D* textureSurface;

	u32	rtWidth;
	u32 rtHeight;

	ID3DDepthStencilView* zBufferInstance;
	ID3DRenderTargetView* renderTargetInstance;
#ifdef USE_DX11
	ID3D11UnorderedAccessView* unorderedAccessViewInstance;
#endif
	ID3DShaderResourceView* shaderResView;
};
#endif

//////////////////////////////////////////////////////////////////////////
class		CRT		:	public xr_resource_named	{
public:
	CRT();
	~CRT();

#if defined (USE_DX11)
private:
	u32			rtWidth;
	u32			rtHeight;
	shared_str	rtName;
public:
	ViewPort	vpStored;
	bool		isTwoViewPorts;


	ID3DTexture2D*				temp;
	ID3DDepthStencilView*		pZRT;
	xr_map<u32, ViewPortRT>		viewPortStuff;
	xr_vector<RtCreationParams>	creationParams;
#endif

#ifdef USE_DX11
	//void	create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount = 1, bool useUAV = false );
	void	create(LPCSTR Name, xr_vector<RtCreationParams>& vp_params, D3DFORMAT f, u32 SampleCount = 1, bool useUAV = false);
	void	SwitchViewPortResources(ViewPort vp);
	u32		RTWidth() { return rtWidth; };
	u32		RTHeight() { return rtHeight; };
	ID3D11UnorderedAccessView* pUAView;
#else
	u32						dwWidth;
	u32						dwHeight;
	void	create(LPCSTR Name, u32 w, u32 h, D3DFORMAT f, u32 SampleCount = 1);
#endif
	void	destroy();
	void	reset_begin();
	void	reset_end();
	IC BOOL	valid()	{ return !!pTexture; }

public:
	ID3DTexture2D*			pSurface;
	ID3DRenderTargetView*	pRT;

	ref_texture				pTexture;

	D3DFORMAT				fmt;

	u64						_order;
};
struct 		resptrcode_crt	: public resptr_base<CRT>
{
#ifdef USE_DX11
	void create(LPCSTR Name, xr_vector<RtCreationParams>& vp_params, D3DFORMAT f, u32 SampleCount = 1, bool useUAV = false);

	void create(LPCSTR Name, RtCreationParams creation_params, D3DFORMAT f, u32 SampleCount = 1, bool useUAV = false)
	{
		xr_vector<RtCreationParams> params;
		params.push_back(creation_params);
		create(Name, params, f, SampleCount, useUAV);
	};

	void create(LPCSTR Name, RtCreationParams creation_params_1, RtCreationParams creation_params_2, D3DFORMAT f, u32 SampleCount = 1, bool useUAV = false)
	{
		xr_vector<RtCreationParams> params;
		params.push_back(creation_params_1);
		params.push_back(creation_params_2);
		create(Name, params, f, SampleCount, useUAV);
	};
#endif //USE_DX11
	void				destroy() { _set(NULL); }
};
typedef	resptr_core<CRT, resptrcode_crt> ref_rt;
#endif // SH_RT_H
