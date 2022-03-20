; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\Environment_render.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

_EPS_L	DD	03a83126fr			; 0.001
PUBLIC	?RenderLast@CEnvironment@@QAEXXZ		; CEnvironment::RenderLast
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\environment_render.cpp
;	COMDAT ?RenderLast@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
?RenderLast@CEnvironment@@QAEXXZ PROC			; CEnvironment::RenderLast, COMDAT
; _this$ = ecx

; 209  : #ifndef _EDITOR
; 210  : 	if (0==g_pGameLevel)			return	;

	cmp	DWORD PTR ?g_pGameLevel@@3PAVIGame_Level@@A, 0 ; g_pGameLevel
	push	esi
	mov	esi, ecx
	je	SHORT $LN9@RenderLast

; 211  : #endif
; 212  : 	// 2
; 213  : 	eff_Rain->Render				();

	mov	eax, DWORD PTR [esi+232]
	mov	ecx, DWORD PTR [eax]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+8]
	call	eax

; 214  : 	eff_Thunderbolt->Render			();

	mov	eax, DWORD PTR [esi+240]
	cmp	DWORD PTR [eax+96], 1
	jne	SHORT $LN9@RenderLast
	mov	ecx, DWORD PTR [eax+92]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+8]
	call	eax
$LN9@RenderLast:
	pop	esi

; 215  : }

	ret	0
?RenderLast@CEnvironment@@QAEXXZ ENDP			; CEnvironment::RenderLast
_TEXT	ENDS
PUBLIC	?RenderFlares@CEnvironment@@QAEXXZ		; CEnvironment::RenderFlares
; Function compile flags: /Ogtpy
;	COMDAT ?RenderFlares@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
?RenderFlares@CEnvironment@@QAEXXZ PROC			; CEnvironment::RenderFlares, COMDAT
; _this$ = ecx

; 200  : #ifndef _EDITOR
; 201  : 	if (0==g_pGameLevel)			return	;

	cmp	DWORD PTR ?g_pGameLevel@@3PAVIGame_Level@@A, 0 ; g_pGameLevel
	je	SHORT $LN9@RenderFlar

; 202  : #endif
; 203  : 	// 1
; 204  : 	eff_LensFlare->Render			(FALSE,TRUE,TRUE);

	mov	eax, DWORD PTR [ecx+236]
	cmp	DWORD PTR [eax+448], 0
	je	SHORT $LN9@RenderFlar
	cmp	DWORD PTR [eax+488], 0
	je	SHORT $LN9@RenderFlar
	mov	ecx, DWORD PTR [eax+472]
	mov	edx, DWORD PTR [ecx]
	push	1
	push	1
	push	0
	push	eax
	mov	eax, DWORD PTR [edx+8]
	call	eax
$LN9@RenderFlar:

; 205  : }

	ret	0
?RenderFlares@CEnvironment@@QAEXXZ ENDP			; CEnvironment::RenderFlares
PUBLIC	?RenderSky@CEnvironment@@QAEXXZ			; CEnvironment::RenderSky
; Function compile flags: /Ogtpy
;	COMDAT ?RenderSky@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
?RenderSky@CEnvironment@@QAEXXZ PROC			; CEnvironment::RenderSky, COMDAT
; _this$ = ecx

; 94   : #ifndef _EDITOR
; 95   : 	if (0==g_pGameLevel)		return;

	cmp	DWORD PTR ?g_pGameLevel@@3PAVIGame_Level@@A, 0 ; g_pGameLevel
	je	SHORT $LN2@RenderSky

; 96   : #endif
; 97   : 
; 98   : 	m_pRender->RenderSky(*this);

	mov	eax, DWORD PTR [ecx+32]
	mov	edx, DWORD PTR [eax]
	push	ecx
	mov	ecx, eax
	mov	eax, DWORD PTR [edx+20]
	call	eax
$LN2@RenderSky:

; 99   : 	/*
; 100  : 	// clouds_sh.create		("clouds","null");
; 101  : 	//. this is the bug-fix for the case when the sky is broken
; 102  : 	//. for some unknown reason the geoms happen to be invalid sometimes
; 103  : 	//. if vTune show this in profile, please add simple cache (move-to-forward last found) 
; 104  : 	//. to the following functions:
; 105  : 	//.		CResourceManager::_CreateDecl
; 106  : 	//.		CResourceManager::CreateGeom
; 107  : 	if(bNeed_re_create_env)
; 108  : 	{
; 109  : 		sh_2sky.create			(&m_b_skybox,"skybox_2t");
; 110  : 		sh_2geom.create			(v_skybox_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
; 111  : 		clouds_sh.create		("clouds","null");
; 112  : 		clouds_geom.create		(v_clouds_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
; 113  : 		bNeed_re_create_env		= FALSE;
; 114  : 	}
; 115  : 	::Render->rmFar				();
; 116  : 
; 117  : 	// draw sky box
; 118  : 	Fmatrix						mSky;
; 119  : 	mSky.rotateY				(CurrentEnv->sky_rotation);
; 120  : 	mSky.translate_over			(Device.vCameraPosition);
; 121  : 
; 122  : 	u32		i_offset,v_offset;
; 123  : 	u32		C					= color_rgba(iFloor(CurrentEnv->sky_color.x*255.f), iFloor(CurrentEnv->sky_color.y*255.f), iFloor(CurrentEnv->sky_color.z*255.f), iFloor(CurrentEnv->weight*255.f));
; 124  : 
; 125  : 	// Fill index buffer
; 126  : 	u16*	pib					= RCache.Index.Lock	(20*3,i_offset);
; 127  : 	CopyMemory					(pib,hbox_faces,20*3*2);
; 128  : 	RCache.Index.Unlock			(20*3);
; 129  : 
; 130  : 	// Fill vertex buffer
; 131  : 	v_skybox* pv				= (v_skybox*)	RCache.Vertex.Lock	(12,sh_2geom.stride(),v_offset);
; 132  : 	for (u32 v=0; v<12; v++)	pv[v].set		(hbox_verts[v*2],C,hbox_verts[v*2+1]);
; 133  : 	RCache.Vertex.Unlock		(12,sh_2geom.stride());
; 134  : 
; 135  : 	// Render
; 136  : 	RCache.set_xform_world		(mSky);
; 137  : 	RCache.set_Geometry			(sh_2geom);
; 138  : 	RCache.set_Shader			(sh_2sky);
; 139  : 	RCache.set_Textures			(&CurrentEnv->sky_r_textures);
; 140  : 	RCache.Render				(D3DPT_TRIANGLELIST,v_offset,0,12,i_offset,20);
; 141  : 
; 142  : 	// Sun
; 143  : 	::Render->rmNormal			();
; 144  : 	eff_LensFlare->Render		(TRUE,FALSE,FALSE);
; 145  : 	*/
; 146  : }

	ret	0
?RenderSky@CEnvironment@@QAEXXZ ENDP			; CEnvironment::RenderSky
PUBLIC	?RenderClouds@CEnvironment@@QAEXXZ		; CEnvironment::RenderClouds
; Function compile flags: /Ogtpy
;	COMDAT ?RenderClouds@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
?RenderClouds@CEnvironment@@QAEXXZ PROC			; CEnvironment::RenderClouds, COMDAT
; _this$ = ecx

; 150  : #ifndef _EDITOR
; 151  : 	if (0==g_pGameLevel)		return	;

	cmp	DWORD PTR ?g_pGameLevel@@3PAVIGame_Level@@A, 0 ; g_pGameLevel
	push	esi
	mov	esi, ecx
	je	SHORT $LN3@RenderClou

; 152  : #endif
; 153  : 	// draw clouds
; 154  : 	if (fis_zero(CurrentEnv->clouds_color.w,EPS_L))	return;

	mov	eax, DWORD PTR [esi+120]
	fld	DWORD PTR [eax+36]
	push	ecx
	fstp	DWORD PTR [esp]
	call	_fabsf
	fld	DWORD PTR _EPS_L
	add	esp, 4
	fcomip	ST(1)
	fstp	ST(0)
	ja	SHORT $LN3@RenderClou

; 155  : 
; 156  : 	m_pRender->RenderClouds(*this);

	mov	ecx, DWORD PTR [esi+32]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+24]
	push	esi
	call	eax
$LN3@RenderClou:
	pop	esi

; 157  : 	/*
; 158  : 
; 159  : 	::Render->rmFar				();
; 160  : 
; 161  : 	Fmatrix						mXFORM, mScale;
; 162  : 	mScale.scale				(10,0.4f,10);
; 163  : 	mXFORM.rotateY				(CurrentEnv->sky_rotation);
; 164  : 	mXFORM.mulB_43				(mScale);
; 165  : 	mXFORM.translate_over		(Device.vCameraPosition);
; 166  : 
; 167  : 	Fvector wd0,wd1;
; 168  : 	Fvector4 wind_dir;
; 169  : 	wd0.setHP					(PI_DIV_4,0);
; 170  : 	wd1.setHP					(PI_DIV_4+PI_DIV_8,0);
; 171  : 	wind_dir.set				(wd0.x,wd0.z,wd1.x,wd1.z).mul(0.5f).add(0.5f).mul(255.f);
; 172  : 	u32		i_offset,v_offset;
; 173  : 	u32		C0					= color_rgba(iFloor(wind_dir.x),iFloor(wind_dir.y),iFloor(wind_dir.w),iFloor(wind_dir.z));
; 174  : 	u32		C1					= color_rgba(iFloor(CurrentEnv->clouds_color.x*255.f),iFloor(CurrentEnv->clouds_color.y*255.f),iFloor(CurrentEnv->clouds_color.z*255.f),iFloor(CurrentEnv->clouds_color.w*255.f));
; 175  : 
; 176  : 	// Fill index buffer
; 177  : 	u16*	pib					= RCache.Index.Lock	(CloudsIndices.size(),i_offset);
; 178  : 	CopyMemory					(pib,&CloudsIndices.front(),CloudsIndices.size()*sizeof(u16));
; 179  : 	RCache.Index.Unlock			(CloudsIndices.size());
; 180  : 
; 181  : 	// Fill vertex buffer
; 182  : 	v_clouds* pv				= (v_clouds*)	RCache.Vertex.Lock	(CloudsVerts.size(),clouds_geom.stride(),v_offset);
; 183  : 	for (FvectorIt it=CloudsVerts.begin(); it!=CloudsVerts.end(); it++,pv++)
; 184  : 		pv->set					(*it,C0,C1);
; 185  : 	RCache.Vertex.Unlock		(CloudsVerts.size(),clouds_geom.stride());
; 186  : 
; 187  : 	// Render
; 188  : 	RCache.set_xform_world		(mXFORM);
; 189  : 	RCache.set_Geometry			(clouds_geom);
; 190  : 	RCache.set_Shader			(clouds_sh);
; 191  : 	RCache.set_Textures			(&CurrentEnv->clouds_r_textures);
; 192  : 	RCache.Render				(D3DPT_TRIANGLELIST,v_offset,0,CloudsVerts.size(),i_offset,CloudsIndices.size()/3);
; 193  : 
; 194  : 	::Render->rmNormal			();
; 195  : 	*/
; 196  : }

	ret	0
?RenderClouds@CEnvironment@@QAEXXZ ENDP			; CEnvironment::RenderClouds
PUBLIC	?OnDeviceDestroy@CEnvironment@@QAEXXZ		; CEnvironment::OnDeviceDestroy
; Function compile flags: /Ogtpy
;	COMDAT ?OnDeviceDestroy@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
?OnDeviceDestroy@CEnvironment@@QAEXXZ PROC		; CEnvironment::OnDeviceDestroy, COMDAT
; _this$ = ecx

; 253  : {

	push	ebx
	push	ebp
	mov	ebp, ecx

; 254  : 	m_pRender->OnDeviceDestroy();

	mov	ecx, DWORD PTR [ebp+32]
	mov	eax, DWORD PTR [ecx]
	mov	edx, DWORD PTR [eax+32]
	push	esi
	push	edi
	call	edx

; 255  : 	/*
; 256  : 	tsky0->surface_set						(NULL);
; 257  : 	tsky1->surface_set						(NULL);
; 258  : 	
; 259  : 	sh_2sky.destroy							();
; 260  : 	sh_2geom.destroy						();
; 261  : 	clouds_sh.destroy						();
; 262  : 	clouds_geom.destroy						();
; 263  : 	*/
; 264  : 	// weathers
; 265  : 	{
; 266  : 		EnvsMapIt _I,_E;
; 267  : 		_I		= WeatherCycles.begin();

	lea	ebx, DWORD PTR [ebp+160]
	mov	ecx, ebx
	call	?_M_leftmost@?$_Rb_tree@Vshared_str@@Ustr_pred@CEnvironment@@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@U?$_Select1st@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@priv@5@U?$_MapTraitsT@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@75@V?$xalloc@U?$pair@Vshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@@@priv@stlp_std@@IAEAAPAU_Rb_tree_node_base@23@XZ ; stlp_std::priv::_Rb_tree<shared_str,CEnvironment::str_pred,stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > >,stlp_std::priv::_Select1st<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,stlp_std::priv::_MapTraitsT<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,xalloc<stlp_std::pair<shared_str,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > > >::_M_leftmost
	mov	esi, DWORD PTR [eax]

; 268  : 		_E		= WeatherCycles.end();
; 269  : 		for (; _I!=_E; _I++)

	cmp	esi, ebx
	setne	al
	test	al, al
	je	$LN10@OnDeviceDe@3
	npad	4
$LL49@OnDeviceDe@3:

; 270  : 			for (EnvIt it=_I->second.begin(); it!=_I->second.end(); it++)

	mov	eax, DWORD PTR [esi+20]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	mov	edi, eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 8
	cmp	edi, eax
	je	SHORT $LN11@OnDeviceDe@3
	npad	5
$LL9@OnDeviceDe@3:

; 271  : 				(*it)->on_device_destroy();

	mov	eax, DWORD PTR [edi]
	mov	ecx, DWORD PTR [eax+20]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+12]
	call	eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	add	edi, 4
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 4
	cmp	edi, eax
	jne	SHORT $LL9@OnDeviceDe@3
$LN11@OnDeviceDe@3:

; 268  : 		_E		= WeatherCycles.end();
; 269  : 		for (; _I!=_E; _I++)

	mov	eax, DWORD PTR [esi+12]
	test	eax, eax
	je	SHORT $LN55@OnDeviceDe@3
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	test	eax, eax
	je	SHORT $LN51@OnDeviceDe@3
	npad	1
$LL59@OnDeviceDe@3:
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	test	eax, eax
	jne	SHORT $LL59@OnDeviceDe@3
	jmp	SHORT $LN51@OnDeviceDe@3
$LN55@OnDeviceDe@3:
	mov	eax, DWORD PTR [esi+4]
	cmp	esi, DWORD PTR [eax+12]
	jne	SHORT $LN52@OnDeviceDe@3
$LL53@OnDeviceDe@3:
	mov	esi, eax
	mov	eax, DWORD PTR [eax+4]
	cmp	esi, DWORD PTR [eax+12]
	je	SHORT $LL53@OnDeviceDe@3
$LN52@OnDeviceDe@3:
	cmp	DWORD PTR [esi+12], eax
	je	SHORT $LN51@OnDeviceDe@3
	mov	esi, eax
$LN51@OnDeviceDe@3:
	cmp	esi, ebx
	setne	al
	test	al, al
	jne	SHORT $LL49@OnDeviceDe@3
$LN10@OnDeviceDe@3:

; 272  : 	}
; 273  : 	// effects
; 274  : 	{
; 275  : 		EnvsMapIt _I,_E;
; 276  : 		_I		= WeatherFXs.begin();

	lea	ebx, DWORD PTR [ebp+184]
	mov	ecx, ebx
	call	?_M_leftmost@?$_Rb_tree@Vshared_str@@Ustr_pred@CEnvironment@@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@U?$_Select1st@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@priv@5@U?$_MapTraitsT@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@75@V?$xalloc@U?$pair@Vshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@@@priv@stlp_std@@IAEAAPAU_Rb_tree_node_base@23@XZ ; stlp_std::priv::_Rb_tree<shared_str,CEnvironment::str_pred,stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > >,stlp_std::priv::_Select1st<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,stlp_std::priv::_MapTraitsT<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,xalloc<stlp_std::pair<shared_str,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > > >::_M_leftmost
	mov	esi, DWORD PTR [eax]

; 277  : 		_E		= WeatherFXs.end();
; 278  : 		for (; _I!=_E; _I++)

	cmp	esi, ebx
	setne	al
	test	al, al
	je	SHORT $LN4@OnDeviceDe@3
$LL128@OnDeviceDe@3:

; 279  : 			for (EnvIt it=_I->second.begin(); it!=_I->second.end(); it++)

	mov	eax, DWORD PTR [esi+20]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	mov	edi, eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 8
	cmp	edi, eax
	je	SHORT $LN5@OnDeviceDe@3
$LL3@OnDeviceDe@3:

; 280  : 				(*it)->on_device_destroy();

	mov	ecx, DWORD PTR [edi]
	mov	ecx, DWORD PTR [ecx+20]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+12]
	call	eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	add	edi, 4
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 4
	cmp	edi, eax
	jne	SHORT $LL3@OnDeviceDe@3
$LN5@OnDeviceDe@3:

; 277  : 		_E		= WeatherFXs.end();
; 278  : 		for (; _I!=_E; _I++)

	mov	eax, DWORD PTR [esi+12]
	test	eax, eax
	je	SHORT $LN134@OnDeviceDe@3
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	test	eax, eax
	je	SHORT $LN130@OnDeviceDe@3
	npad	1
$LL138@OnDeviceDe@3:
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	test	eax, eax
	jne	SHORT $LL138@OnDeviceDe@3
	jmp	SHORT $LN130@OnDeviceDe@3
$LN134@OnDeviceDe@3:
	mov	eax, DWORD PTR [esi+4]
	cmp	esi, DWORD PTR [eax+12]
	jne	SHORT $LN131@OnDeviceDe@3
$LL132@OnDeviceDe@3:
	mov	esi, eax
	mov	eax, DWORD PTR [eax+4]
	cmp	esi, DWORD PTR [eax+12]
	je	SHORT $LL132@OnDeviceDe@3
$LN131@OnDeviceDe@3:
	cmp	DWORD PTR [esi+12], eax
	je	SHORT $LN130@OnDeviceDe@3
	mov	esi, eax
$LN130@OnDeviceDe@3:
	cmp	esi, ebx
	setne	al
	test	al, al
	jne	SHORT $LL128@OnDeviceDe@3
$LN4@OnDeviceDe@3:

; 281  : 	}
; 282  : 	CurrentEnv->destroy();

	mov	esi, DWORD PTR [ebp+120]
	mov	ecx, DWORD PTR [esi+188]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+8]
	call	eax
	mov	ecx, DWORD PTR [esi+20]
	mov	edx, DWORD PTR [ecx]
	mov	eax, DWORD PTR [edx+12]
	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	jmp	eax
?OnDeviceDestroy@CEnvironment@@QAEXXZ ENDP		; CEnvironment::OnDeviceDestroy
PUBLIC	?OnDeviceCreate@CEnvironment@@QAEXXZ		; CEnvironment::OnDeviceCreate
; Function compile flags: /Ogtpy
;	COMDAT ?OnDeviceCreate@CEnvironment@@QAEXXZ
_TEXT	SEGMENT
_this$ = -4						; size = 4
?OnDeviceCreate@CEnvironment@@QAEXXZ PROC		; CEnvironment::OnDeviceCreate, COMDAT
; _this$ = ecx

; 218  : {

	push	ecx
	push	ebx
	push	ebp
	push	esi
	push	edi
	mov	edi, ecx

; 219  : //.	bNeed_re_create_env			= TRUE;
; 220  : 	m_pRender->OnDeviceCreate();

	mov	ecx, DWORD PTR [edi+32]
	mov	eax, DWORD PTR [ecx]
	mov	edx, DWORD PTR [eax+28]
	mov	DWORD PTR _this$[esp+20], edi
	call	edx

; 221  : 	/*
; 222  : 	sh_2sky.create			(&m_b_skybox,"skybox_2t");
; 223  : 	sh_2geom.create			(v_skybox_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
; 224  : 	clouds_sh.create		("clouds","null");
; 225  : 	clouds_geom.create		(v_clouds_fvf,RCache.Vertex.Buffer(), RCache.Index.Buffer());
; 226  : 	*/
; 227  : 
; 228  : 	// weathers
; 229  : 	{
; 230  : 		EnvsMapIt _I,_E;
; 231  : 		_I		= WeatherCycles.begin();

	lea	ebp, DWORD PTR [edi+160]
	mov	ecx, ebp
	call	?_M_leftmost@?$_Rb_tree@Vshared_str@@Ustr_pred@CEnvironment@@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@U?$_Select1st@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@priv@5@U?$_MapTraitsT@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@75@V?$xalloc@U?$pair@Vshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@@@priv@stlp_std@@IAEAAPAU_Rb_tree_node_base@23@XZ ; stlp_std::priv::_Rb_tree<shared_str,CEnvironment::str_pred,stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > >,stlp_std::priv::_Select1st<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,stlp_std::priv::_MapTraitsT<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,xalloc<stlp_std::pair<shared_str,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > > >::_M_leftmost
	mov	esi, DWORD PTR [eax]

; 232  : 		_E		= WeatherCycles.end();
; 233  : 		for (; _I!=_E; _I++)

	cmp	esi, ebp
	setne	al
	xor	ebx, ebx
	cmp	al, bl
	je	$LN10@OnDeviceCr@4
$LL49@OnDeviceCr@4:

; 234  : 			for (EnvIt it=_I->second.begin(); it!=_I->second.end(); it++)

	mov	eax, DWORD PTR [esi+20]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	mov	edi, eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 8
	cmp	edi, eax
	je	SHORT $LN11@OnDeviceCr@4
	npad	2
$LL9@OnDeviceCr@4:

; 235  : 				(*it)->on_device_create();

	mov	eax, DWORD PTR [edi]
	mov	ecx, DWORD PTR [eax+20]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+8]
	call	eax
	mov	eax, DWORD PTR [esi+24]
	push	eax
	add	edi, 4
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 4
	cmp	edi, eax
	jne	SHORT $LL9@OnDeviceCr@4
$LN11@OnDeviceCr@4:

; 232  : 		_E		= WeatherCycles.end();
; 233  : 		for (; _I!=_E; _I++)

	mov	eax, DWORD PTR [esi+12]
	cmp	eax, ebx
	je	SHORT $LN55@OnDeviceCr@4
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	cmp	eax, ebx
	je	SHORT $LN51@OnDeviceCr@4
$LL59@OnDeviceCr@4:
	mov	esi, eax
	mov	eax, DWORD PTR [esi+8]
	cmp	eax, ebx
	jne	SHORT $LL59@OnDeviceCr@4
	jmp	SHORT $LN51@OnDeviceCr@4
$LN55@OnDeviceCr@4:
	mov	eax, DWORD PTR [esi+4]
	cmp	esi, DWORD PTR [eax+12]
	jne	SHORT $LN52@OnDeviceCr@4
$LL53@OnDeviceCr@4:
	mov	esi, eax
	mov	eax, DWORD PTR [eax+4]
	cmp	esi, DWORD PTR [eax+12]
	je	SHORT $LL53@OnDeviceCr@4
$LN52@OnDeviceCr@4:
	cmp	DWORD PTR [esi+12], eax
	je	SHORT $LN51@OnDeviceCr@4
	mov	esi, eax
$LN51@OnDeviceCr@4:
	cmp	esi, ebp
	setne	al
	cmp	al, bl
	jne	SHORT $LL49@OnDeviceCr@4
	mov	edi, DWORD PTR _this$[esp+20]
$LN10@OnDeviceCr@4:

; 236  : 	}
; 237  : 	// effects
; 238  : 	{
; 239  : 		EnvsMapIt _I,_E;
; 240  : 		_I		= WeatherFXs.begin();

	lea	ebp, DWORD PTR [edi+184]
	mov	ecx, ebp
	call	?_M_leftmost@?$_Rb_tree@Vshared_str@@Ustr_pred@CEnvironment@@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@U?$_Select1st@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@priv@5@U?$_MapTraitsT@U?$pair@$$CBVshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@75@V?$xalloc@U?$pair@Vshared_str@@V?$xr_vector@PAVCEnvDescriptor@@V?$xalloc@PAVCEnvDescriptor@@@@@@@stlp_std@@@@@priv@stlp_std@@IAEAAPAU_Rb_tree_node_base@23@XZ ; stlp_std::priv::_Rb_tree<shared_str,CEnvironment::str_pred,stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > >,stlp_std::priv::_Select1st<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,stlp_std::priv::_MapTraitsT<stlp_std::pair<shared_str const ,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > >,xalloc<stlp_std::pair<shared_str,xr_vector<CEnvDescriptor *,xalloc<CEnvDescriptor *> > > > >::_M_leftmost
	mov	edi, DWORD PTR [eax]

; 241  : 		_E		= WeatherFXs.end();
; 242  : 		for (; _I!=_E; _I++)

	cmp	edi, ebp
	setne	al
	cmp	al, bl
	je	$LN4@OnDeviceCr@4
	npad	3
$LL128@OnDeviceCr@4:

; 243  : 			for (EnvIt it=_I->second.begin(); it!=_I->second.end(); it++)

	mov	eax, DWORD PTR [edi+20]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	mov	esi, eax
	mov	eax, DWORD PTR [edi+24]
	push	eax
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 8
	cmp	esi, eax
	je	SHORT $LN5@OnDeviceCr@4
	npad	5
$LL3@OnDeviceCr@4:

; 244  : 				(*it)->on_device_create();

	mov	eax, DWORD PTR [esi]
	mov	ecx, DWORD PTR [eax+20]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+8]
	call	eax
	mov	eax, DWORD PTR [edi+24]
	push	eax
	add	esi, 4
	call	?cv_ptr@?$_VoidCastTraitsAux@PAXPAX@priv@stlp_std@@SAPAPAXPAPAX@Z ; stlp_std::priv::_VoidCastTraitsAux<void *,void *>::cv_ptr
	add	esp, 4
	cmp	esi, eax
	jne	SHORT $LL3@OnDeviceCr@4
$LN5@OnDeviceCr@4:

; 241  : 		_E		= WeatherFXs.end();
; 242  : 		for (; _I!=_E; _I++)

	mov	eax, DWORD PTR [edi+12]
	cmp	eax, ebx
	je	SHORT $LN134@OnDeviceCr@4
	mov	edi, eax
	mov	eax, DWORD PTR [edi+8]
	cmp	eax, ebx
	je	SHORT $LN130@OnDeviceCr@4
$LL138@OnDeviceCr@4:
	mov	edi, eax
	mov	eax, DWORD PTR [edi+8]
	cmp	eax, ebx
	jne	SHORT $LL138@OnDeviceCr@4
	jmp	SHORT $LN130@OnDeviceCr@4
$LN134@OnDeviceCr@4:
	mov	eax, DWORD PTR [edi+4]
	cmp	edi, DWORD PTR [eax+12]
	jne	SHORT $LN131@OnDeviceCr@4
$LL132@OnDeviceCr@4:
	mov	edi, eax
	mov	eax, DWORD PTR [eax+4]
	cmp	edi, DWORD PTR [eax+12]
	je	SHORT $LL132@OnDeviceCr@4
$LN131@OnDeviceCr@4:
	cmp	DWORD PTR [edi+12], eax
	je	SHORT $LN130@OnDeviceCr@4
	mov	edi, eax
$LN130@OnDeviceCr@4:
	cmp	edi, ebp
	setne	al
	cmp	al, bl
	jne	SHORT $LL128@OnDeviceCr@4
$LN4@OnDeviceCr@4:

; 245  : 	}
; 246  : 
; 247  : 
; 248  : 	Invalidate	();

	mov	ecx, DWORD PTR _this$[esp+20]
	mov	eax, DWORD PTR [ecx+236]
	cmp	eax, ebx
	mov	BYTE PTR [ecx+132], bl
	mov	DWORD PTR [ecx+124], ebx
	mov	DWORD PTR [ecx+128], ebx
	je	SHORT $LN178@OnDeviceCr@4
	mov	DWORD PTR [eax+492], ebx
$LN178@OnDeviceCr@4:

; 250  : }

	pop	edi
	pop	esi
	pop	ebp
	pop	ebx
	add	esp, 4

; 249  : 	OnFrame		();

	jmp	?OnFrame@CEnvironment@@QAEXXZ		; CEnvironment::OnFrame
?OnDeviceCreate@CEnvironment@@QAEXXZ ENDP		; CEnvironment::OnDeviceCreate
END