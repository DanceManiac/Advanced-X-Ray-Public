//---------------------------------


#ifdef __cplusplus
extern "C" {
#endif

	class xrIDirect3DSurface9: public IDirect3DSurface9	
	{
	protected:
		
		LONG		m_refCount;
		IDirect3DDevice9*	m_pIDirect3DDevice9;
		BYTE*		m_pLockedData;
	public:
		xrIDirect3DSurface9	(IDirect3DDevice9* pIDirect3DDevice9, UINT Width,UINT Height,D3DFORMAT Format,D3DMULTISAMPLE_TYPE MultiSample,DWORD MultisampleQuality);
		/*** IUnknown methods ***/
		HRESULT				QueryInterface( REFIID riid, void** ppvObj) ;
		ULONG				AddRef() ;
		ULONG				Release() ;

		/*** IDirect3DResource9 methods ***/
		HRESULT				GetDevice( IDirect3DDevice9** ppDevice) ;
		HRESULT				SetPrivateData( REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) ;
		HRESULT				GetPrivateData( REFGUID refguid,void* pData,DWORD* pSizeOfData) ;
		HRESULT				FreePrivateData( REFGUID refguid) ;
		DWORD				SetPriority( DWORD PriorityNew) ;
		DWORD				GetPriority() ;
		void				PreLoad() ;
		D3DRESOURCETYPE		GetType() ;
		HRESULT				GetContainer( REFIID riid,void** ppContainer) ;
		HRESULT				GetDesc( D3DSURFACE_DESC *pDesc) ;
		HRESULT				LockRect( D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags) ;
		HRESULT				UnlockRect() ;
		HRESULT				GetDC( HDC *phdc) ;
		HRESULT				ReleaseDC( HDC hdc) ;

//#ifdef D3D_DEBUG_INFO
		LPCWSTR Name;
		UINT Width;
		UINT Height;
		DWORD Usage;
		D3DFORMAT Format;
		D3DPOOL Pool;
		D3DMULTISAMPLE_TYPE MultiSampleType;
		DWORD MultiSampleQuality;
		DWORD Priority;
		UINT LockCount;
		UINT DCCount;
		LPCWSTR CreationCallStack;
//#endif
	};

#ifdef __cplusplus
};
#endif