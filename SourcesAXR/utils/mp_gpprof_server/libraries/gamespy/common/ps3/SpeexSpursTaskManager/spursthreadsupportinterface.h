// Gamespy Technology
// NOTE:  this code has been provided by Sony for usage in Speex SPURS Manager

/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://bulletphysics.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/

#ifndef THREAD_SUPPORT_INTERFACE_H
#define THREAD_SUPPORT_INTERFACE_H

#include "spursPlatformDefinitions.h"

class spursThreadSupportInterface
{
public:

	virtual ~spursThreadSupportInterface();

///send messages to SPUs
	virtual int sendRequest(uint32_t uiCommand, uint32_t uiArgument0, uint32_t uiArgument1) =0;

///check for messages from SPUs
	virtual	int waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1) =0;

///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual	int startSPU() =0;

///tell the task scheduler we are done with the SPU tasks
	virtual	int stopSPU()=0;
};

#endif //THREAD_SUPPORT_INTERFACE_H
