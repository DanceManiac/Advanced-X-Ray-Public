; Listing generated by Microsoft (R) Optimizing Compiler Version 14.00.50727.762 

	TITLE	D:\CLEARSKY\sources\engine\xrEngine\Render.cpp
	.686P
	.XMM
	include listing.inc
	.model	flat

INCLUDELIB OLDNAMES

PUBLIC	??1IRender_Glow@@UAE@XZ				; IRender_Glow::~IRender_Glow
; Function compile flags: /Ogtpy
; File d:\clearsky\sources\engine\xrengine\render.cpp
;	COMDAT ??1IRender_Glow@@UAE@XZ
_TEXT	SEGMENT
??1IRender_Glow@@UAE@XZ PROC				; IRender_Glow::~IRender_Glow, COMDAT
; _this$ = ecx

; 12   : IRender_Glow::~IRender_Glow		()			{	

	mov	eax, ecx

; 13   : 	::Render->glow_destroy		(this	);	

	mov	ecx, DWORD PTR __imp_?Render@@3PAVIRender_interface@@A
	mov	DWORD PTR [eax], OFFSET ??_7IRender_Glow@@6B@
	mov	ecx, DWORD PTR [ecx]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+136]
	call	eax

; 14   : }

	ret	0
??1IRender_Glow@@UAE@XZ ENDP				; IRender_Glow::~IRender_Glow
_TEXT	ENDS
PUBLIC	??1IRender_Light@@UAE@XZ			; IRender_Light::~IRender_Light
; Function compile flags: /Ogtpy
;	COMDAT ??1IRender_Light@@UAE@XZ
_TEXT	SEGMENT
??1IRender_Light@@UAE@XZ PROC				; IRender_Light::~IRender_Light, COMDAT
; _this$ = ecx

; 9    : IRender_Light::~IRender_Light	()			{	

	mov	eax, ecx

; 10   : 	::Render->light_destroy		(this	);

	mov	ecx, DWORD PTR __imp_?Render@@3PAVIRender_interface@@A
	mov	DWORD PTR [eax], OFFSET ??_7IRender_Light@@6B@
	mov	ecx, DWORD PTR [ecx]
	mov	edx, DWORD PTR [ecx]
	push	eax
	mov	eax, DWORD PTR [edx+128]
	call	eax

; 11   : }

	ret	0
??1IRender_Light@@UAE@XZ ENDP				; IRender_Light::~IRender_Light
_TEXT	ENDS
PUBLIC	??1IRender_interface@@UAE@XZ			; IRender_interface::~IRender_interface
; Function compile flags: /Ogtpy
;	COMDAT ??1IRender_interface@@UAE@XZ
_TEXT	SEGMENT
??1IRender_interface@@UAE@XZ PROC			; IRender_interface::~IRender_interface, COMDAT
; _this$ = ecx

; 4    : IRender_interface::~IRender_interface()		{};

	mov	DWORD PTR [ecx], OFFSET ??_7IRender_interface@@6B@
	ret	0
??1IRender_interface@@UAE@XZ ENDP			; IRender_interface::~IRender_interface
END