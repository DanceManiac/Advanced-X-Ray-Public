//---------------------------------
#include <stdlib.h>
#include <objbase.h>
#include <windows.h>
//---------------------------------

#include "d3d9.h"

#ifdef __cplusplus
extern "C" {
#endif

	class xrIDirect3DDevice9: public IDirect3DDevice9
	{
	protected:

		LONG		m_refCount;
		IDirect3D9*	m_pIDirect3D9;
	public:
		xrIDirect3DDevice9(IDirect3D9* pDirect3D9, D3DPRESENT_PARAMETERS* pPresentationParameters);
		/*** IUnknown methods ***/
		HRESULT			QueryInterface( REFIID riid, void** ppvObj);
		ULONG			AddRef();
		ULONG			Release();

		HRESULT			TestCooperativeLevel();

		UINT			GetAvailableTextureMem() ;
		HRESULT			EvictManagedResources() ;
		HRESULT			GetDirect3D( IDirect3D9** ppD3D9) ;
		HRESULT			GetDeviceCaps( D3DCAPS9* pCaps) ;
		HRESULT			GetDisplayMode( UINT iSwapChain,D3DDISPLAYMODE* pMode) ;
		HRESULT			GetCreationParameters( D3DDEVICE_CREATION_PARAMETERS *pParameters) ;
		HRESULT			SetCursorProperties( UINT XHotSpot,UINT YHotSpot,IDirect3DSurface9* pCursorBitmap) ;
		void			SetCursorPosition( int X,int Y,DWORD Flags) ;
		BOOL			ShowCursor( BOOL bShow) ;
		HRESULT			CreateAdditionalSwapChain( D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DSwapChain9** pSwapChain) ;
		HRESULT			GetSwapChain( UINT iSwapChain,IDirect3DSwapChain9** pSwapChain) ;
		UINT			GetNumberOfSwapChains() ;
		HRESULT			Reset( D3DPRESENT_PARAMETERS* pPresentationParameters) ;
		HRESULT			Present( CONST RECT* pSourceRect,CONST RECT* pDestRect,HWND hDestWindowOverride,CONST RGNDATA* pDirtyRegion) ;
		HRESULT			GetBackBuffer( UINT iSwapChain,UINT iBackBuffer,D3DBACKBUFFER_TYPE Type,IDirect3DSurface9** ppBackBuffer) ;
		HRESULT			GetRasterStatus( UINT iSwapChain,D3DRASTER_STATUS* pRasterStatus) ;
		HRESULT			SetDialogBoxMode( BOOL bEnableDialogs) ;
		void			SetGammaRamp( UINT iSwapChain,DWORD Flags,CONST D3DGAMMARAMP* pRamp) ;
		void			GetGammaRamp( UINT iSwapChain,D3DGAMMARAMP* pRamp) ;
		HRESULT			CreateTexture( UINT Width,UINT Height,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DTexture9** ppTexture,HANDLE* pSharedHandle) ;
		HRESULT			CreateVolumeTexture( UINT Width,UINT Height,UINT Depth,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DVolumeTexture9** ppVolumeTexture,HANDLE* pSharedHandle) ;
		HRESULT			CreateCubeTexture( UINT EdgeLength,UINT Levels,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DCubeTexture9** ppCubeTexture,HANDLE* pSharedHandle) ;
		HRESULT			CreateVertexBuffer( UINT Length,DWORD Usage,DWORD FVF,D3DPOOL Pool,IDirect3DVertexBuffer9** ppVertexBuffer,HANDLE* pSharedHandle) ;
		HRESULT			CreateIndexBuffer( UINT Length,DWORD Usage,D3DFORMAT Format,D3DPOOL Pool,IDirect3DIndexBuffer9** ppIndexBuffer,HANDLE* pSharedHandle) ;
		HRESULT			CreateRenderTarget( UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Lockable,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) ;
		HRESULT			CreateDepthStencilSurface( UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality,BOOL Discard,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) ;
		HRESULT			UpdateSurface( IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestinationSurface,CONST POINT* pDestPoint) ;
		HRESULT			UpdateTexture( IDirect3DBaseTexture9* pSourceTexture,IDirect3DBaseTexture9* pDestinationTexture) ;
		HRESULT			GetRenderTargetData( IDirect3DSurface9* pRenderTarget,IDirect3DSurface9* pDestSurface) ;
		HRESULT			GetFrontBufferData( UINT iSwapChain,IDirect3DSurface9* pDestSurface) ;
		HRESULT			StretchRect( IDirect3DSurface9* pSourceSurface,CONST RECT* pSourceRect,IDirect3DSurface9* pDestSurface,CONST RECT* pDestRect,D3DTEXTUREFILTERTYPE Filter) ;
		HRESULT			ColorFill( IDirect3DSurface9* pSurface,CONST RECT* pRect,D3DCOLOR color) ;
		HRESULT			CreateOffscreenPlainSurface( UINT Width,UINT Height,D3DFORMAT Format,D3DPOOL Pool,IDirect3DSurface9** ppSurface,HANDLE* pSharedHandle) ;
		HRESULT			SetRenderTarget( DWORD RenderTargetIndex,IDirect3DSurface9* pRenderTarget) ;
		HRESULT			GetRenderTarget( DWORD RenderTargetIndex,IDirect3DSurface9** ppRenderTarget) ;
		HRESULT			SetDepthStencilSurface( IDirect3DSurface9* pNewZStencil) ;
		HRESULT			GetDepthStencilSurface( IDirect3DSurface9** ppZStencilSurface) ;
		HRESULT			BeginScene() ;
		HRESULT			EndScene() ;
		HRESULT			Clear( DWORD Count,CONST D3DRECT* pRects,DWORD Flags,D3DCOLOR Color,float Z,DWORD Stencil) ;
		HRESULT			SetTransform( D3DTRANSFORMSTATETYPE State,CONST D3DMATRIX* pMatrix) ;
		HRESULT			GetTransform( D3DTRANSFORMSTATETYPE State,D3DMATRIX* pMatrix) ;
		HRESULT			MultiplyTransform( D3DTRANSFORMSTATETYPE,CONST D3DMATRIX*) ;
		HRESULT			SetViewport( CONST D3DVIEWPORT9* pViewport) ;
		HRESULT			GetViewport( D3DVIEWPORT9* pViewport) ;
		HRESULT			SetMaterial( CONST D3DMATERIAL9* pMaterial) ;
		HRESULT			GetMaterial( D3DMATERIAL9* pMaterial) ;
		HRESULT			SetLight( DWORD Index,CONST D3DLIGHT9*) ;
		HRESULT			GetLight( DWORD Index,D3DLIGHT9*) ;
		HRESULT			LightEnable( DWORD Index,BOOL Enable) ;
		HRESULT			GetLightEnable( DWORD Index,BOOL* pEnable) ;
		HRESULT			SetClipPlane( DWORD Index,CONST float* pPlane) ;
		HRESULT			GetClipPlane( DWORD Index,float* pPlane) ;
		HRESULT			SetRenderState( D3DRENDERSTATETYPE State,DWORD Value) ;
		HRESULT			GetRenderState( D3DRENDERSTATETYPE State,DWORD* pValue) ;
		HRESULT			CreateStateBlock( D3DSTATEBLOCKTYPE Type,IDirect3DStateBlock9** ppSB) ;
		HRESULT			BeginStateBlock() ;
		HRESULT			EndStateBlock( IDirect3DStateBlock9** ppSB) ;
		HRESULT			SetClipStatus( CONST D3DCLIPSTATUS9* pClipStatus) ;
		HRESULT			GetClipStatus( D3DCLIPSTATUS9* pClipStatus) ;
		HRESULT			GetTexture( DWORD Stage,IDirect3DBaseTexture9** ppTexture) ;
		HRESULT			SetTexture( DWORD Stage,IDirect3DBaseTexture9* pTexture) ;
		HRESULT			GetTextureStageState( DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD* pValue) ;
		HRESULT			SetTextureStageState( DWORD Stage,D3DTEXTURESTAGESTATETYPE Type,DWORD Value) ;
		HRESULT			GetSamplerState( DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD* pValue) ;
		HRESULT			SetSamplerState( DWORD Sampler,D3DSAMPLERSTATETYPE Type,DWORD Value) ;
		HRESULT			ValidateDevice( DWORD* pNumPasses) ;
		HRESULT			SetPaletteEntries( UINT PaletteNumber,CONST PALETTEENTRY* pEntries) ;
		HRESULT			GetPaletteEntries( UINT PaletteNumber,PALETTEENTRY* pEntries) ;
		HRESULT			SetCurrentTexturePalette( UINT PaletteNumber) ;
		HRESULT			GetCurrentTexturePalette( UINT *PaletteNumber) ;
		HRESULT			SetScissorRect( CONST RECT* pRect) ;
		HRESULT			GetScissorRect( RECT* pRect) ;
		HRESULT			SetSoftwareVertexProcessing( BOOL bSoftware) ;
		BOOL			GetSoftwareVertexProcessing() ;
		HRESULT			SetNPatchMode( float nSegments) ;
		float			GetNPatchMode() ;
		HRESULT			DrawPrimitive( D3DPRIMITIVETYPE PrimitiveType,UINT StartVertex,UINT PrimitiveCount) ;
		HRESULT			DrawIndexedPrimitive( D3DPRIMITIVETYPE,INT BaseVertexIndex,UINT MinVertexIndex,UINT NumVertices,UINT startIndex,UINT primCount) ;
		HRESULT			DrawPrimitiveUP( D3DPRIMITIVETYPE PrimitiveType,UINT PrimitiveCount,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride) ;
		HRESULT			DrawIndexedPrimitiveUP( D3DPRIMITIVETYPE PrimitiveType,UINT MinVertexIndex,UINT NumVertices,UINT PrimitiveCount,CONST void* pIndexData,D3DFORMAT IndexDataFormat,CONST void* pVertexStreamZeroData,UINT VertexStreamZeroStride) ;
		HRESULT			ProcessVertices( UINT SrcStartIndex,UINT DestIndex,UINT VertexCount,IDirect3DVertexBuffer9* pDestBuffer,IDirect3DVertexDeclaration9* pVertexDecl,DWORD Flags) ;
		HRESULT			CreateVertexDeclaration( CONST D3DVERTEXELEMENT9* pVertexElements,IDirect3DVertexDeclaration9** ppDecl) ;
		HRESULT			SetVertexDeclaration( IDirect3DVertexDeclaration9* pDecl) ;
		HRESULT			GetVertexDeclaration( IDirect3DVertexDeclaration9** ppDecl) ;
		HRESULT			SetFVF( DWORD FVF) ;
		HRESULT			GetFVF( DWORD* pFVF) ;
		HRESULT			CreateVertexShader( CONST DWORD* pFunction,IDirect3DVertexShader9** ppShader) ;
		HRESULT			SetVertexShader( IDirect3DVertexShader9* pShader) ;
		HRESULT			GetVertexShader( IDirect3DVertexShader9** ppShader) ;
		HRESULT			SetVertexShaderConstantF( UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount) ;
		HRESULT			GetVertexShaderConstantF( UINT StartRegister,float* pConstantData,UINT Vector4fCount) ;
		HRESULT			SetVertexShaderConstantI( UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount) ;
		HRESULT			GetVertexShaderConstantI( UINT StartRegister,int* pConstantData,UINT Vector4iCount) ;
		HRESULT			SetVertexShaderConstantB( UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount) ;
		HRESULT			GetVertexShaderConstantB( UINT StartRegister,BOOL* pConstantData,UINT BoolCount) ;
		HRESULT			SetStreamSource( UINT StreamNumber,IDirect3DVertexBuffer9* pStreamData,UINT OffsetInBytes,UINT Stride) ;
		HRESULT			GetStreamSource( UINT StreamNumber,IDirect3DVertexBuffer9** ppStreamData,UINT* pOffsetInBytes,UINT* pStride) ;
		HRESULT			SetStreamSourceFreq( UINT StreamNumber,UINT Setting) ;
		HRESULT			GetStreamSourceFreq( UINT StreamNumber,UINT* pSetting) ;
		HRESULT			SetIndices( IDirect3DIndexBuffer9* pIndexData) ;
		HRESULT			GetIndices( IDirect3DIndexBuffer9** ppIndexData) ;
		HRESULT			CreatePixelShader( CONST DWORD* pFunction,IDirect3DPixelShader9** ppShader) ;
		HRESULT			SetPixelShader( IDirect3DPixelShader9* pShader) ;
		HRESULT			GetPixelShader( IDirect3DPixelShader9** ppShader) ;
		HRESULT			SetPixelShaderConstantF( UINT StartRegister,CONST float* pConstantData,UINT Vector4fCount) ;
		HRESULT			GetPixelShaderConstantF( UINT StartRegister,float* pConstantData,UINT Vector4fCount) ;
		HRESULT			SetPixelShaderConstantI( UINT StartRegister,CONST int* pConstantData,UINT Vector4iCount) ;
		HRESULT			GetPixelShaderConstantI( UINT StartRegister,int* pConstantData,UINT Vector4iCount) ;
		HRESULT			SetPixelShaderConstantB( UINT StartRegister,CONST BOOL* pConstantData,UINT  BoolCount) ;
		HRESULT			GetPixelShaderConstantB( UINT StartRegister,BOOL* pConstantData,UINT BoolCount) ;
		HRESULT			DrawRectPatch( UINT Handle,CONST float* pNumSegs,CONST D3DRECTPATCH_INFO* pRectPatchInfo) ;
		HRESULT			DrawTriPatch( UINT Handle,CONST float* pNumSegs,CONST D3DTRIPATCH_INFO* pTriPatchInfo) ;
		HRESULT			DeletePatch( UINT Handle) ;
		HRESULT			CreateQuery( D3DQUERYTYPE Type,IDirect3DQuery9** ppQuery) ;
		
//#ifdef D3D_DEBUG_INFO
		D3DDEVICE_CREATION_PARAMETERS CreationParameters;
		D3DPRESENT_PARAMETERS PresentParameters;
		D3DDISPLAYMODE DisplayMode;
		D3DCAPS9 Caps;


		UINT AvailableTextureMem;
		UINT SwapChains;
		UINT Textures;
		UINT VertexBuffers;
		UINT IndexBuffers;
		UINT VertexShaders;
		UINT PixelShaders;

		D3DVIEWPORT9 Viewport;
		D3DMATRIX ProjectionMatrix;
		D3DMATRIX ViewMatrix;
		D3DMATRIX WorldMatrix;
		D3DMATRIX TextureMatrices[8];

		DWORD FVF;
		UINT VertexSize;
		DWORD VertexShaderVersion;
		DWORD PixelShaderVersion;
		BOOL SoftwareVertexProcessing;

		D3DMATERIAL9 Material;
		D3DLIGHT9 Lights[16];
		BOOL LightsEnabled[16];

		D3DGAMMARAMP GammaRamp;
		RECT ScissorRect;
		BOOL DialogBoxMode;
//#endif
	};

#ifdef __cplusplus
};
#endif