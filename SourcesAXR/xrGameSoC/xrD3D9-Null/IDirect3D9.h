//---------------------------------
#include <stdlib.h>
#include <objbase.h>
#include <windows.h>

#include "d3d9.h"

/*
//---------------------------------
#include "d3d9types.h"
#include "d3d9caps.h"

#include "IDirect3DDevice9.h"
*/
/* IID_IDirect3D9 */
/* {81BDCBCA-64D4-426d-AE8D-AD0147F4275C} */
//DEFINE_GUID(IID_IDirect3D9, 0x81bdcbca, 0x64d4, 0x426d, 0xae, 0x8d, 0xad, 0x1, 0x47, 0xf4, 0x27, 0x5c);

//interface DECLSPEC_UUID("81BDCBCA-64D4-426d-AE8D-AD0147F4275C") IDirect3D9;

//typedef interface IDirect3D9                    IDirect3D9;



#ifdef __cplusplus
extern "C" {
#endif


	class xrIDirect3D9: public IDirect3D9
	{
	protected:

		LONG		m_refCount;
	public:
		xrIDirect3D9();
		/*** IUnknown methods ***/
		HRESULT			QueryInterface( REFIID riid, void** ppvObj);
		ULONG			AddRef();
		ULONG			Release();

		/*** IDirect3D9 methods ***/
		HRESULT			RegisterSoftwareDevice( void* pInitializeFunction);
		UINT			GetAdapterCount();
		HRESULT			GetAdapterIdentifier( UINT Adapter,DWORD Flags,D3DADAPTER_IDENTIFIER9* pIdentifier);
		UINT			GetAdapterModeCount( UINT Adapter,D3DFORMAT Format);
		HRESULT			EnumAdapterModes( UINT Adapter,D3DFORMAT Format,UINT Mode,D3DDISPLAYMODE* pMode);
		HRESULT			GetAdapterDisplayMode( UINT Adapter,D3DDISPLAYMODE* pMode);
		HRESULT			CheckDeviceType( UINT Adapter,D3DDEVTYPE DevType,D3DFORMAT AdapterFormat,D3DFORMAT BackBufferFormat,BOOL bWindowed);
		HRESULT			CheckDeviceFormat( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,DWORD Usage,D3DRESOURCETYPE RType,D3DFORMAT CheckFormat);
		HRESULT			CheckDeviceMultiSampleType( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SurfaceFormat,BOOL Windowed,D3DMULTISAMPLE_TYPE MultiSampleType,DWORD* pQualityLevels);
		HRESULT			CheckDepthStencilMatch( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT AdapterFormat,D3DFORMAT RenderTargetFormat,D3DFORMAT DepthStencilFormat);
		HRESULT			CheckDeviceFormatConversion( UINT Adapter,D3DDEVTYPE DeviceType,D3DFORMAT SourceFormat,D3DFORMAT TargetFormat);
		HRESULT			GetDeviceCaps( UINT Adapter,D3DDEVTYPE DeviceType,D3DCAPS9* pCaps);
		HMONITOR		GetAdapterMonitor( UINT Adapter);
		HRESULT			CreateDevice( UINT Adapter,D3DDEVTYPE DeviceType,HWND hFocusWindow,DWORD BehaviorFlags,D3DPRESENT_PARAMETERS* pPresentationParameters,IDirect3DDevice9** ppReturnedDeviceInterface);

///#ifdef D3D_DEBUG_INFO
		LPCWSTR Version;
//#endif
	};


#ifdef __cplusplus
};
#endif


