#include "StdAfx.h"
#include "UIArtefactPanel.h"
#include "UIInventoryUtilities.h"
#include "UIXmlInit.h"

#include "../Artefact.h"

using namespace InventoryUtilities;

CUIArtefactPanel::CUIArtefactPanel()
{		
	m_cell_size.set(UI().inv_grid_kx(), UI().inv_grid_kx());
	m_fScale = 1.0f;
}

CUIArtefactPanel::~CUIArtefactPanel()
{
}

void CUIArtefactPanel::InitFromXML	(CUIXml& xml, LPCSTR path, int index)
{
	CUIXmlInit::InitWindow		(xml, path, index, this);
	m_cell_size.x				= xml.ReadAttribFlt(path, index, "cell_width", UI().inv_grid_kx());
	m_cell_size.y				= xml.ReadAttribFlt(path, index, "cell_height", UI().inv_grid_kx());
	m_fScale					= xml.ReadAttribFlt(path, index, "scale", 1.0f);
}

void CUIArtefactPanel::InitIcons(const xr_vector<const CArtefact*>& artefacts)
{
	m_vRects.clear();
	m_si.SetShader(InventoryUtilities::GetEquipmentIconsShader());
	
	for (xr_vector<const CArtefact*>::const_iterator it = artefacts.begin(); it != artefacts.end(); it++)
	{
		const CArtefact* artefact = *it;

		Frect rect;
		rect.left = pSettings->r_float(artefact->cNameSect(), "inv_grid_x") * UI().inv_grid_kx();
		rect.top = pSettings->r_float(artefact->cNameSect(), "inv_grid_y") * UI().inv_grid_kx();
		rect.right = rect.left + pSettings->r_float(artefact->cNameSect(), "inv_grid_width") * UI().inv_grid_kx();
		rect.bottom = rect.top + pSettings->r_float(artefact->cNameSect(), "inv_grid_height") * UI().inv_grid_kx();
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

	for (const Frect& r : m_vRects)
	{
		Fvector2 size;

		iHeight = m_fScale * (r.bottom - r.top) * (1 / UI().get_icons_kx());
		iWidth = _s * m_fScale * (r.right - r.left) * (1 / UI().get_icons_kx());

		m_si.SetOriginalRect(r.left, r.top, r.width(), r.height());
		m_si.SetRect(0, 0, iWidth, iHeight);

		m_si.SetPos(x, y);
		m_si.Render();
	}

	CUIWindow::Draw();
}
