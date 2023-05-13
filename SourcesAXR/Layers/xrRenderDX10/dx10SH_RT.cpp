#include "stdafx.h"
#pragma hdrstop

#include "../xrRender/ResourceManager.h"

#include "../xrRender/dxRenderDeviceRender.h"

#include "dx10TextureUtils.h"

CRT::CRT			()
{
	pSurface		= NULL;
	pRT				= NULL;
	pZRT			= NULL;
	pUAView			= NULL;
	//dwWidth		= 0;
	//dwHeight		= 0;
	rtWidth			= 0;
	rtHeight		= 0;
	fmt				= D3DFMT_UNKNOWN;
	vpStored		= (ViewPort)0;
}
CRT::~CRT			()
{
	destroy			();

	// release external reference
	DEV->_DeleteRT	(this);
}

void CRT::create(LPCSTR Name, xr_vector<RtCreationParams>& vp_params, D3DFORMAT f, u32 SampleCount, bool useUAV)
{
	if (valid()) return;

	rtName = Name;

	R_ASSERT(HW.pDevice && Name && Name[0]);
	_order		= CPU::GetCLK()	;	//Device.GetTimerGlobal()->GetElapsed_clk();

	//HRESULT		_hr;

	creationParams = vp_params; // for device reset

	//dwWidth	= w;
	//dwHeight	= h;
	fmt			= f;

	// Get caps
	//D3DCAPS9	caps;
	//R_CHK		(HW.pDevice->GetDeviceCaps(&caps));

	//	DirectX 10 supports non-power of two textures
	// Pow2
	//if (!btwIsPow2(w) || !btwIsPow2(h))
	//{
	//	if (!HW.Caps.raster.bNonPow2)	return;
	//}

	// Check width-and-height of render target surface
	//if (w>D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION)		return;
	//if (h>D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION)		return;

	// Select usage
	u32 usage	= 0;
	if (D3DFMT_D24X8==fmt)									usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D24S8		==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D15S1		==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16			==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D16_LOCKABLE==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if (D3DFMT_D32F_LOCKABLE==fmt)						usage = D3DUSAGE_DEPTHSTENCIL;
	else if ((D3DFORMAT)MAKEFOURCC('D','F','2','4') == fmt)	usage = D3DUSAGE_DEPTHSTENCIL;
	else													usage = D3DUSAGE_RENDERTARGET;


	DXGI_FORMAT dx10FMT;
   
   if( fmt != D3DFMT_D24S8 )
      dx10FMT = dx10TextureUtils::ConvertTextureFormat(fmt);
   else
      {
      dx10FMT = DXGI_FORMAT_R24G8_TYPELESS;
      usage = D3DUSAGE_DEPTHSTENCIL;
      }

	bool	bUseAsDepth = (usage == D3DUSAGE_RENDERTARGET)?false:true;

	//DEV->Evict				();

	// Try to create texture/surface
	DEV->Evict				();
	//_hr = HW.pDevice->CreateTexture		(w, h, 1, usage, f, D3DPOOL_DEFAULT, &pSurface,NULL);
	//if (FAILED(_hr) || (0==pSurface))	return;
	// Create the render target texture
	D3D_TEXTURE2D_DESC desc;
	ZeroMemory( &desc, sizeof(desc) );
	desc.Width = 0; // dwWidth;
	desc.Height = 0; // dwHeight;
	desc.MipLevels = 1;
	desc.ArraySize = 1;
	desc.Format = dx10FMT;
	desc.SampleDesc.Count = SampleCount;
	desc.Usage = D3D_USAGE_DEFAULT;
   if( SampleCount <= 1 )
	   desc.BindFlags = D3D_BIND_SHADER_RESOURCE | (bUseAsDepth ? D3D_BIND_DEPTH_STENCIL : D3D_BIND_RENDER_TARGET);
   else
   {
      desc.BindFlags = (bUseAsDepth ? D3D_BIND_DEPTH_STENCIL : (D3D_BIND_SHADER_RESOURCE | D3D_BIND_RENDER_TARGET));
      if( RImplementation.o.dx10_msaa_opt )
      {
         desc.SampleDesc.Quality = UINT(D3D_STANDARD_MULTISAMPLE_PATTERN);
      }
   }

	if (HW.FeatureLevel>=D3D_FEATURE_LEVEL_11_0 && !bUseAsDepth && SampleCount == 1 && useUAV )
		desc.BindFlags |= D3D11_BIND_UNORDERED_ACCESS;

	//CHK_DX( HW.pDevice->CreateTexture2D( &desc, NULL, &pSurface ) );
	//HW.stats_manager.increment_stats_rtarget( pSurface );
	// OK
#ifdef DEBUG
	//Msg			("* created RT(%s), %dx%d, format = %d samples = %d",Name,w,h, dx10FMT, SampleCount );
#endif // DEBUG
	//R_CHK		(pSurface->GetSurfaceLevel	(0,&pRT));

	D3D_DEPTH_STENCIL_VIEW_DESC	ViewDesc;

	if (bUseAsDepth)
	{
		//D3D_DEPTH_STENCIL_VIEW_DESC	ViewDesc;
		ZeroMemory( &ViewDesc, sizeof(ViewDesc) );

		ViewDesc.Format = DXGI_FORMAT_UNKNOWN;
		if( SampleCount <= 1 )
		{
			ViewDesc.ViewDimension = D3D_DSV_DIMENSION_TEXTURE2D;
		}
		else
		{
			ViewDesc.ViewDimension = D3D_DSV_DIMENSION_TEXTURE2DMS;
			ViewDesc.Texture2DMS.UnusedField_NothingToDefine = 0;
		}

		ViewDesc.Texture2D.MipSlice = 0;
		switch (desc.Format)
		{
		case DXGI_FORMAT_R24G8_TYPELESS:
			ViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
			break;
		case DXGI_FORMAT_R32_TYPELESS:
			ViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
			break;
		}

		//CHK_DX( HW.pDevice->CreateDepthStencilView( pSurface, &ViewDesc, &pZRT) );
	}
	//else
		//CHK_DX( HW.pDevice->CreateRenderTargetView( pSurface, 0, &pRT ) );

	pTexture = DEV->_CreateTexture(Name);

	for (size_t i = 0; i < vp_params.size(); ++i)
	{
		R_ASSERT(vp_params[i].w && vp_params[i].h);

		// Check width-and-height of render target surface
		if (vp_params[i].w > D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION)
			return;
		if (vp_params[i].h > D3D_REQ_TEXTURE2D_U_OR_V_DIMENSION)
			return;

		desc.Width = vp_params[i].w;
		desc.Height = vp_params[i].h;

		auto it = viewPortStuff.insert(std::make_pair(vp_params[i].viewport, ViewPortRT()));

		it.first->second.rtWidth = vp_params[i].w;
		it.first->second.rtHeight = vp_params[i].h;

		// Try to create texture/surface
		DEV->Evict();

		R_CHK(HW.pDevice->CreateTexture2D(&desc, NULL, &it.first->second.textureSurface));

		HW.stats_manager.increment_stats_rtarget(it.first->second.textureSurface);

		if (bUseAsDepth)
			R_CHK(HW.pDevice->CreateDepthStencilView(it.first->second.textureSurface, &ViewDesc, &it.first->second.zBufferInstance));
		else
			R_CHK(HW.pDevice->CreateRenderTargetView(it.first->second.textureSurface, 0, &it.first->second.renderTargetInstance));


		if (HW.FeatureLevel >= D3D_FEATURE_LEVEL_11_0 && !bUseAsDepth && SampleCount == 1 && useUAV)
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC UAVDesc;
			ZeroMemory(&UAVDesc, sizeof(D3D11_UNORDERED_ACCESS_VIEW_DESC));
			UAVDesc.Format = dx10FMT;
			UAVDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
			UAVDesc.Buffer.FirstElement = 0;
			UAVDesc.Buffer.NumElements = vp_params[i].w * vp_params[i].h;
			CHK_DX(HW.pDevice->CreateUnorderedAccessView(pSurface, &UAVDesc, &pUAView));
		}

		Msg("Create resource for %s", rtName.c_str());

		it.first->second.shaderResView = pTexture->CreateShaderRes(it.first->second.textureSurface);

		//pTexture

		//R_ASSERT2(it.first->second.shaderResView, make_string("%s", rtName.c_str()));

		it.first->second.textureSurface->AddRef();

		if (pTexture->Description().SampleDesc.Count <= 1 || pTexture->Description().Format != DXGI_FORMAT_R24G8_TYPELESS)
			R_ASSERT2(it.first->second.shaderResView, make_string("%s", rtName.c_str()));
	}

	auto it = viewPortStuff.begin();

	pRT = it->second.renderTargetInstance;
	pZRT = it->second.zBufferInstance;
	pUAView = it->second.unorderedAccessViewInstance;
	pSurface = it->second.textureSurface;
	rtWidth = it->second.rtWidth;
	rtHeight = it->second.rtHeight;
	pTexture->SurfaceSetRT(it->second.textureSurface, it->second.shaderResView);

}

void CRT::destroy()
{
	if (pTexture._get())
	{
		//pTexture->surface_set	(0);
		pTexture->surface_null();
		pTexture.destroy();
		pTexture = nullptr;
	}
	for (auto it = viewPortStuff.begin(); it != viewPortStuff.end(); it++)
	{
		_RELEASE(it->second.renderTargetInstance);
		_RELEASE(it->second.zBufferInstance);

		HW.stats_manager.decrement_stats_rtarget(it->second.textureSurface);

		_RELEASE(it->second.textureSurface);
		_RELEASE(it->second.unorderedAccessViewInstance);
		_RELEASE(it->second.shaderResView);
	}
}
void CRT::reset_begin	()
{
	destroy		();
}
void CRT::reset_end		()
{
	create(*cName, creationParams, fmt);
}

void CRT::SwitchViewPortResources(ViewPort vp)
{
	if (vpStored == vp && pSurface)
		return;

	vpStored = vp;

	xr_map<u32, ViewPortRT>::iterator it = viewPortStuff.find(vp);

	if (it == viewPortStuff.end())
	{
		it = viewPortStuff.find(MAIN_VIEWPORT);
	}

	R_ASSERT(it != viewPortStuff.end());

	const ViewPortRT& value = it->second;

	pRT = value.renderTargetInstance;
	pZRT = value.zBufferInstance;
	pUAView = value.unorderedAccessViewInstance;
	pSurface = value.textureSurface;
	rtWidth = value.rtWidth;
	rtHeight = value.rtHeight;

	R_ASSERT2(pRT || pZRT, make_string("%s", rtName.c_str()));
	R_ASSERT2(pSurface, make_string("%s", rtName.c_str()));
	//R_ASSERT2(value.shaderResView, make_string("%s", rtName.c_str()));

	if (pTexture->Description().SampleDesc.Count <= 1 || pTexture->Description().Format != DXGI_FORMAT_R24G8_TYPELESS)
		R_ASSERT2(value.shaderResView, make_string("%s", rtName.c_str()));


	pTexture->SurfaceSetRT(value.textureSurface, value.shaderResView);
}

void resptrcode_crt::create(LPCSTR Name, xr_vector<RtCreationParams>& vp_params, D3DFORMAT f, u32 SampleCount, bool useUAV)
{
	_set(DEV->_CreateRT(Name, vp_params, f, SampleCount, useUAV));
}