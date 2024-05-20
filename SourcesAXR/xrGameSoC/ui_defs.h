#ifndef	ui_defs_included
#define	ui_defs_included
#pragma once

typedef	FactoryPtr<IUIShader>	ui_shader;

enum EUIItemAlign {
	alNone = 0x0000,
	alLeft = 0x0001,
	alRight = 0x0002,
	alTop = 0x0004,
	alBottom = 0x0008,
	alCenter = 0x0010
};

#endif	//	ui_defs_included