#include "stdafx.h"
#include "resource.h"
#include "dedicated_server_only.h"

#ifdef INGAME_EDITOR
#	include "../include/editor/ide.hpp"
#	include "engine_impl.hpp"
#endif // #ifdef INGAME_EDITOR

extern LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

#ifdef INGAME_EDITOR
void CRenderDevice::initialize_editor	()
{
	m_editor_module		= LoadLibrary("editor.dll");
	if (!m_editor_module) {
		Msg				("! cannot load library \"editor.dll\"");
		return;
	}

	m_editor_initialize	= (initialize_function_ptr)GetProcAddress(m_editor_module, "initialize");
	VERIFY				(m_editor_initialize);

	m_editor_finalize	= (finalize_function_ptr)GetProcAddress(m_editor_module, "finalize");
	VERIFY				(m_editor_finalize);

	m_engine			= xr_new<engine_impl>();
	m_editor_initialize	(m_editor, m_engine);
	VERIFY				(m_editor);

	m_hWnd				= m_editor->view_handle();
	VERIFY				(m_hWnd != INVALID_HANDLE_VALUE);
}
#endif // #ifdef INGAME_EDITOR

PROTECT_API void CRenderDevice::Initialize()
{
	Log("Initializing Engine...");
	TimerGlobal.Start();
	TimerMM.Start();

	// Unless a substitute hWnd has been specified, create a window to render into
	if (m_hWnd == NULL)
	{
		LPCSTR    wndclass = "Advanced X-Ray";

		// Register the windows class
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(0);
		WNDCLASS wndClass = { CS_HREDRAW | CS_VREDRAW | CS_OWNDC, WndProc, 0, 0, hInstance,
		  LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)),
		  LoadCursor(NULL, IDC_ARROW), (HBRUSH)GetStockObject(BLACK_BRUSH), NULL, wndclass };
		RegisterClass(&wndClass);

		// Set the window's initial style
		m_dwWindowStyle = WS_CLIPSIBLINGS | WS_CLIPCHILDREN | WS_POPUP; // WS_BORDER | WS_DLGFRAME;

		u32 screen_width = GetSystemMetrics(SM_CXSCREEN);
		u32 screen_height = GetSystemMetrics(SM_CYSCREEN);

		DEVMODE screen_settings;
		memset(&screen_settings, 0, sizeof(screen_settings));
		screen_settings.dmSize = sizeof(screen_settings);
		screen_settings.dmPelsWidth = (unsigned long)screen_width;
		screen_settings.dmPelsHeight = (unsigned long)screen_height;
		screen_settings.dmBitsPerPel = 32;
		screen_settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

		ChangeDisplaySettings(&screen_settings, CDS_FULLSCREEN);

		m_hWnd = CreateWindowEx(WS_EX_TOPMOST, wndclass, "S.T.A.L.K.E.R.: Call of Pripyat", m_dwWindowStyle, 0, 0, screen_width, screen_height, 0L, 0, hInstance, 0L);
}

	// Save window properties
	m_dwWindowStyle = GetWindowLong(m_hWnd, GWL_STYLE);
	GetWindowRect(m_hWnd, &m_rcWindowBounds);
	GetClientRect(m_hWnd, &m_rcWindowClient);
}

