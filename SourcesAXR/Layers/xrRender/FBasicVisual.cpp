// dxRender_Visual.cpp: implementation of the dxRender_Visual class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#pragma hdrstop

#ifndef _EDITOR
#	include "../../xrEngine/render.h"
#endif // #ifndef _EDITOR

#include "fbasicvisual.h"
#include "../../xrEngine/fmesh.h"
#include <filesystem>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

IRender_Mesh::~IRender_Mesh()		
{ 
	_RELEASE(p_rm_Vertices); 
	_RELEASE(p_rm_Indices);		
}

dxRender_Visual::dxRender_Visual		()
{
	Type				= 0;
	shader				= 0;
	vis.clear			();
}

dxRender_Visual::~dxRender_Visual		()
{
}

void dxRender_Visual::Release		()
{
}

static bool replaceShadersLine(const char* N, char* fnS, u32 fnS_size, LPCSTR item)
{
    if (!pAdvancedSettings->line_exist("vis_shaders_replace", item))
        return false;

    LPCSTR overrides = pAdvancedSettings->r_string("vis_shaders_replace", item);
    u32 cnt = _GetItemCount(overrides);
    R_ASSERT(cnt % 2 == 0, "[%s]: vis_shaders_replace: wrong format cnt = %u: %s = %s", __FUNCTION__, cnt, item, overrides);

    for (u32 i = 0; i < cnt; i += 2)
    {
        string256 s1, s2;
        _GetItem(overrides, i, s1);
        _GetItem(overrides, i + 1, s2);
        if (xr_strcmp(s1, fnS) == 0)
        {
            xr_strcpy(fnS, fnS_size, s2);
            break;
        }
    }

    return true;
}

static bool replaceShaders(const char* N, char* fnS, u32 fnS_size)
{
    if (!pSettings->section_exist("vis_shaders_replace"))
        return false;

    if (replaceShadersLine(N, fnS, fnS_size, N))
        return true;

    if (strchr(N, ':'))
    {
        std::string s(N);
        s.erase(s.find(":"));
        if (replaceShadersLine(N, fnS, fnS_size, s.c_str()))
            return true;
    }

    std::filesystem::path p = N;
    while (p.has_parent_path())
    {
        p = p.parent_path();
        if (replaceShadersLine(N, fnS, fnS_size, p.string().c_str()))
            return true;
    }

    return false;
}

void dxRender_Visual::Load		(const char* N, IReader *data, u32 )
{
#ifdef DEBUG
	dbg_name	= N;
#endif

	// header
	VERIFY		(data);
	ogf_header	hdr;
	if (data->r_chunk_safe(OGF_HEADER,&hdr,sizeof(hdr)))
	{
		R_ASSERT2			(hdr.format_version==xrOGF_FormatVersion, "Invalid visual version");
		Type				= hdr.type;
		//if (hdr.shader_id)	shader	= ::Render->getShader	(hdr.shader_id);
		if (hdr.shader_id)	shader	= ::RImplementation.getShader	(hdr.shader_id);
		vis.box.set			(hdr.bb.min,hdr.bb.max	);
		vis.sphere.set		(hdr.bs.c,	hdr.bs.r	);
	} else {
		FATAL				("Invalid visual");
	}

	// Shader
	if (data->find_chunk(OGF_TEXTURE)) {
		string256		fnT,fnS;
		data->r_stringZ	(fnT,sizeof(fnT));
		data->r_stringZ	(fnS,sizeof(fnS));
        if (replaceShaders(N, fnS, sizeof fnS)) 
        {
            //Msg("~~[%s] replaced shaders for [%s]: %s", __FUNCTION__, N, fnS);
        }
		shader.create	(fnS,fnT);
	}

    // desc
#ifdef _EDITOR
    if (data->find_chunk(OGF_S_DESC)) 
	    desc.Load		(*data);
#endif
}

#define PCOPY(a)	a = pFrom->a
void	dxRender_Visual::Copy(dxRender_Visual *pFrom)
{
	PCOPY(Type);
	PCOPY(shader);
	PCOPY(vis);
#ifdef _EDITOR
	PCOPY(desc);
#endif
#ifdef DEBUG
	PCOPY(dbg_name);
#endif
}
