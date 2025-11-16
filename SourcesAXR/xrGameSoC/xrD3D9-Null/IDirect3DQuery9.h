//---------------------------------


#ifdef __cplusplus
extern "C" {
#endif


	class xrIDirect3DQuery9: public IDirect3DQuery9
	{
	protected:

		LONG		m_refCount;
		IDirect3DDevice9*	m_pIDirect3DDevice9;
	public:
		xrIDirect3DQuery9(IDirect3DDevice9* pIDirect3DDevice9, D3DQUERYTYPE rType);
		/*** IUnknown methods ***/
		HRESULT			QueryInterface( REFIID riid, void** ppvObj) ;
		ULONG			AddRef() ;
		ULONG			Release() ;

		/*** IDirect3DQuery9 methods ***/
		HRESULT				GetDevice( IDirect3DDevice9** ppDevice) ;
		D3DQUERYTYPE		GetType() ;
		DWORD				GetDataSize() ;
		HRESULT				Issue( DWORD dwIssueFlags) ;
		HRESULT				GetData( void* pData,DWORD dwSize,DWORD dwGetDataFlags) ;

//#ifdef D3D_DEBUG_INFO
		D3DQUERYTYPE Type;
		DWORD DataSize;
		LPCWSTR CreationCallStack;
//#endif
	};


#ifdef __cplusplus
};
#endif
