#include "stdafx.h"
#pragma hdrstop

#include "SoundRender_Core.h"
#include "SoundRender_Source.h"

CSoundRender_Source*	CSoundRender_Core::i_create_source		(LPCSTR name)
{
	// Search
	string256			id;
	xr_strcpy(id,name)	;
	strlwr				(id);
	if (strext(id))		*strext(id) = 0;
	for (u32 it=0; it<s_sources.size(); it++)		{
		if (0==xr_strcmp(*s_sources[it]->fname,id))	return s_sources[it];
	}

	// Load a _new one
	CSoundRender_Source* S	= xr_new<CSoundRender_Source>	();
	S->load					(id);
	s_sources.push_back		(S);
	return S;
}

void CSoundRender_Core::i_create_all_sources()
{
	CTimer T;
	T.Start();

	FS_FileSet flist;
	FS.file_list(flist, "$game_sounds$", FS_ListFiles, "*.ogg");
	for (const FS_File& file : flist)
	{
		string256 id;
		xr_strcpy(id, file.name.c_str());

		xr_strlwr(id);
		if (strext(id))
			*strext(id) = 0;

		CSoundRender_Source* S = new CSoundRender_Source();
		if (!S->load(id, false, false))
		{
			xr_delete(S);
			continue;
		}
		s_sources.push_back(S);
	}

	Msg("Finished creating %d sound sources. Duration: %d ms", flist.size(), T.GetElapsed_ms());
}