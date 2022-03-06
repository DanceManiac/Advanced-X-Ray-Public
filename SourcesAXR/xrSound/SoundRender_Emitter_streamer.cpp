#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_Core.h"
#include "SoundRender_Source.h"
#include "SoundRender_Emitter.h"
#include "SoundRender_Target.h"

void	CSoundRender_Emitter::fill_data(u8* _dest, u32 offset, u32 size)
{
	u32		line_size						= SoundRender->cache.get_linesize();
	u32		line							= offset / line_size;

	// prepare for first line (it can be unaligned)
	u32		line_offs						= offset - line*line_size;
	u32		line_amount						= line_size - line_offs;
	
	while	(size)
	{
		// cache access
		if (SoundRender->cache.request(source()->CAT,line))		
		{
			source()->decompress	(line,target->get_data());
		}
                                                
		// fill block
		u32		blk_size	= _min(size,line_amount);
		u8*		ptr			= (u8*)SoundRender->cache.get_dataptr(source()->CAT,line);
		CopyMemory			(_dest,ptr+line_offs,blk_size);
		
		// advance
		line		++	;
		size		-=	blk_size;
		_dest		+=	blk_size;
		offset		+=	blk_size;
		line_offs	=	0;
		line_amount	=	line_size;
	}
}

void	CSoundRender_Emitter::fill_block	(void* ptr, u32 size)
{
	LPBYTE		dest = LPBYTE(ptr);
	u32 dwBytesTotal = get_bytes_total();

	if ( (get_cursor(true)+size) > dwBytesTotal)
	{
		// We are reaching the end of data, what to do?
		switch (m_current_state)
		{
		case stPlaying:
			{	// Fill as much data as we can, zeroing remainder
				if(get_cursor(true) >= dwBytesTotal)
				{
					// ??? We requested the block after remainder - just zero
					Memory.mem_fill	(dest,0,size);
				}else 
				{
					// Calculate remainder
					u32	sz_data		= dwBytesTotal - get_cursor(true);
					u32 sz_zero		= (get_cursor(true)+size) - dwBytesTotal;
					VERIFY			(size == (sz_data+sz_zero));
					fill_data		(dest,get_cursor(false),sz_data);
					Memory.mem_fill	(dest+sz_data,0,sz_zero);
				}
				move_cursor					(size);
			}
			break;
		case stPlayingLooped:
			{
            	u32 hw_position				= 0;
				do{
					u32	sz_data				= dwBytesTotal - get_cursor(true);
                    u32 sz_write			= _min(size-hw_position,sz_data);
					fill_data				(dest+hw_position,	get_cursor(true),	sz_write);
                    hw_position				+= sz_write;
					move_cursor				(sz_write);
                    set_cursor				(get_cursor(true) % dwBytesTotal );
                }while(0!=(size-hw_position));
			}
			break;
		default:
			FATAL					("SOUND: Invalid emitter state");
			break;
		}
	} else {

		u32 bt_handle	= ((CSoundRender_Source*)owner_data->handle)->dwBytesTotal;
		if(get_cursor(true)+size > m_cur_handle_cursor+bt_handle)
		{
			R_ASSERT					(owner_data->fn_attached[0].size());
			
			u32 rem = 0;
			if( (m_cur_handle_cursor+bt_handle) > get_cursor(true) )
			{
				rem						= (m_cur_handle_cursor+bt_handle) - get_cursor(true);

				fill_data				(dest, get_cursor(false), rem);
				move_cursor				(rem);
			}
			fill_block					(dest+rem,	size-rem);
		}
		else
		{
			fill_data					(dest, get_cursor(false), size);
			move_cursor					(size);
		}
	}
}

u32	CSoundRender_Emitter::get_bytes_total()	const	
{
	return owner_data->dwBytesTotal;
}

float CSoundRender_Emitter::get_length_sec() const
{
	return owner_data->get_length_sec();
}