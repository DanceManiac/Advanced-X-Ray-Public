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
	m_si.SetShader(GetEquipmentIconsShader());
	m_vRects.clear();
	
	for (const CArtefact* artefact : artefacts)
	{
		Frect rect;
		rect.x1 = pSettings->r_float(artefact->cNameSect(), "inv_grid_x") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
		rect.y1 = pSettings->r_float(artefact->cNameSect(), "inv_grid_y") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
		rect.x2 = pSettings->r_float(artefact->cNameSect(), "inv_grid_width") * INV_GRID_WIDTH(GameConstants::GetUseHQ_Icons());
		rect.y2 = pSettings->r_float(artefact->cNameSect(), "inv_grid_height") * INV_GRID_HEIGHT(GameConstants::GetUseHQ_Icons());
		rect.rb.add(rect.lt);

		m_vRects.push_back(rect);
	}
}

void CUIArtefactPanel::Draw()
{
	float x = 0.0f;
	float y = 0.0f;

	Frect				rect;
	GetAbsoluteRect		(rect);
	x					= rect.left;
	y					= rect.top;	
	
	float _s			= m_cell_size.x/m_cell_size.y;

	for (const Frect& r : m_vRects)
	{
		Fvector2 size;

		if (GameConstants::GetUseHQ_Icons())
		{
			size.x = m_fScale * (r.bottom - r.top) * UI().get_current_kx() / 2;
			size.y = _s * m_fScale * (r.right - r.left) / 2;
		}
		else
		{
			size.x = m_fScale * (r.bottom - r.top) * UI().get_current_kx();
			size.y = _s * m_fScale * (r.right - r.left);
		}

		m_si.SetTextureRect(r);
		m_si.SetSize(size);

		m_si.SetPos(x, y);

		if (!m_bVert)
			x = x + m_fIndent + size.x;
		else
			y = y + m_fIndent + size.y;

        m_si.Render();
	}

	CUIWindow::Draw();
}
