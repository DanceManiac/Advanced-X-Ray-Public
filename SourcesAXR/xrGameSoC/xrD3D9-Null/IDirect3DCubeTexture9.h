

#ifdef __cplusplus
extern "C" {
#endif

	class xrIDirect3DCubeTexture9: public IDirect3DCubeTexture9
	{
	protected:

		LONG		m_refCount;
		IDirect3DDevice9*	m_pIDirect3DDevice9;
	public:
		xrIDirect3DCubeTexture9(IDirect3DDevice9*	pIDirect3DDevice9, UINT iWidth,UINT iHeight,UINT iLevels,DWORD iUsage,D3DFORMAT iFormat,D3DPOOL iPool);
		/*** IUnknown methods ***/
		HRESULT				QueryInterface( REFIID riid, void** ppvObj) ;
		ULONG				AddRef() ;
		ULONG				Release() ;

		/*** IDirect3DBaseTexture9 methods ***/
		HRESULT				GetDevice( IDirect3DDevice9** ppDevice) ;
		HRESULT				SetPrivateData( REFGUID refguid,CONST void* pData,DWORD SizeOfData,DWORD Flags) ;
		HRESULT				GetPrivateData( REFGUID refguid,void* pData,DWORD* pSizeOfData) ;
		HRESULT				FreePrivateData( REFGUID refguid) ;
		DWORD				SetPriority( DWORD PriorityNew) ;
		DWORD				GetPriority() ;
		void				PreLoad() ;
		D3DRESOURCETYPE		GetType() ;
		DWORD				SetLOD( DWORD LODNew) ;
		DWORD				GetLOD() ;
		DWORD				GetLevelCount() ;
		HRESULT				SetAutoGenFilterType( D3DTEXTUREFILTERTYPE FilterType) ;
		D3DTEXTUREFILTERTYPE				GetAutoGenFilterType() ;
		void				GenerateMipSubLevels() ;
		HRESULT				GetLevelDesc( UINT Level,D3DSURFACE_DESC *pDesc) ;
		HRESULT				GetCubeMapSurface( D3DCUBEMAP_FACES FaceType,UINT Level,IDirect3DSurface9** ppCubeMapSurface);
		HRESULT				LockRect( D3DCUBEMAP_FACES FaceType, UINT Level,D3DLOCKED_RECT* pLockedRect,CONST RECT* pRect,DWORD Flags) ;
		HRESULT				UnlockRect( D3DCUBEMAP_FACES FaceType, UINT Level) ;
		HRESULT				AddDirtyRect( D3DCUBEMAP_FACES FaceType, CONST RECT* pDirtyRect) ;

		//#ifdef D3D_DEBUG_INFO
		LPCWSTR Name;
		UINT Width;
		UINT Height;
		UINT Levels;
		DWORD Usage;
		D3DFORMAT Format;
		D3DPOOL Pool;
		DWORD Priority;
		DWORD LOD;
		D3DTEXTUREFILTERTYPE FilterType;
		UINT LockCount;
		LPCWSTR CreationCallStack;
		//#endif
	};

#ifdef __cplusplus
};
#endif