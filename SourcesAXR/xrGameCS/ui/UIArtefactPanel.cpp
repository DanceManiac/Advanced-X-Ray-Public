#include "StdAfx.h"
#include "UIArtefactPanel.h"
#include "UIInventoryUtilities.h"
#include "UIXmlInit.h"

#include "../Artefact.h"

using namespace InventoryUtilities;

CUIArtefactPanel::CUIArtefactPanel()
{		
	m_cell_size.set(50.f, 50.f);
	m_fScale = 1.0f;
	m_bVert = false;
	m_bShowInInventory = false;
	m_fIndent = 1.0f;
}

CUIArtefactPanel::~CUIArtefactPanel()
{
}

void CUIArtefactPanel::InitFromXML	(CUIXml& xml, LPCSTR path, int index)
{
	CUIXmlInit::InitWindow		(xml, path, index, this);
	m_cell_size.x				= xml.ReadAttribFlt(path, index, "cell_width", 50.0f);
	m_cell_size.y				= xml.ReadAttribFlt(path, index, "cell_height", 50.0f);
	m_fScale					= xml.ReadAttribFlt(path, index, "scale", 1.0f);
	m_bVert						= xml.ReadAttribInt(path, index, "vert", 0) == 1;
	m_bShowInInventory			= xml.ReadAttribInt(path, index, "show_in_inv", 0) == 1;
	m_fIndent					= xml.ReadAttribFlt(path, index, "indent", 1.0f);
}

void CUIArtefactPanel::InitIcons(const xr_vector<const CArtefact*>& artefacts)
{
	m_vRects.clear();
	m_si.SetShader(InventoryUtilities::GetEquipmentIconsShader());

	for(xr_vector<const CArtefact*>::const_iterator it = artefacts.begin(); it != artefacts.end(); it++)
	{
		const CArtefact* artefact = *it;

		Frect rect;
		rect.left = pSettings->r_float(artefact->cNameSect(), "inv_grid_x") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
		rect.top = pSettings->r_float(artefact->cNameSect(), "inv_grid_y") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
		rect.right = rect.left + pSettings->r_float(artefact->cNameSect(), "inv_grid_width") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
		rect.bottom = rect.top + pSettings->r_float(artefact->cNameSect(), "inv_grid_height") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
		m_vRects.push_back(rect);
	}
}

void CUIArtefactPanel::Draw()
{
	float x = 0.0f;
	float y = 0.0f;
	float iHeight;
	float iWidth;

	Frect				rect;
	GetAbsoluteRect(rect);
	x = rect.left;
	y = rect.top;

	float _s = m_cell_size.x / m_cell_size.y;

	for (ITr it = m_vRects.begin(); it != m_vRects.end(); ++it)
	{
		const Frect& r = *it;

		iHeight = m_fScale * (r.bottom - r.top);
		iWidth = _s * m_fScale * (r.right - r.left);

		if (m_bVert)
			y = y + m_fIndent + iHeight;
		else
			x = x + m_fIndent + iWidth;

		m_si.SetOriginalRect(r.left, r.top, r.width(), r.height());
		m_si.SetRect(0, 0, iWidth, iHeight);

		m_si.SetPos(x, y);
		m_si.Render();
	}

	CUIWindow::Draw();
}
