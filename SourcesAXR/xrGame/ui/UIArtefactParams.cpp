#include "stdafx.h"
#include "UIArtefactParams.h"
#include "UIStatic.h"

#include "..\actor.h"
#include "..\ActorCondition.h"
#include "object_broker.h"
#include "UIXmlInit.h"
#include "UIHelper.h"
#include "../string_table.h"
#include "../Inventory_Item.h"
#include "../Artefact.h"
#include "../AdvancedXrayGameConstants.h"

u32 const red_clr   = color_argb(255,210,50,50);
u32 const green_clr = color_argb(255,170,170,170);

CUIArtefactParams::CUIArtefactParams()
{
	for ( u32 i = 0; i < ALife::eHitTypeMax; ++i )
	{
		m_immunity_item[i] = NULL;
	}
	for (u32 i = 0; i < ALife::eRestoreTypeMax; ++i)
	{
		m_restore_item[i] = NULL;
	}
	m_fWalkAccel = NULL;
	m_fJumpSpeed = NULL;
	m_additional_weight = NULL;
	m_iArtefactRank = NULL;
	m_fChargeLevel = NULL;
}

CUIArtefactParams::~CUIArtefactParams()
{
	delete_data	(m_immunity_item);
	delete_data	(m_restore_item);
	xr_delete	(m_fWalkAccel);
	xr_delete	(m_fJumpSpeed);
	xr_delete	(m_additional_weight);
	xr_delete	(m_iArtefactRank);
	xr_delete	(m_fChargeLevel);
	xr_delete	(m_Prop_line);
}

LPCSTR af_immunity_section_names[] = // ALife::EHitType
{
	"burn_immunity",			// eHitTypeBurn=0
	"shock_immunity",			// eHitTypeShock=1
	"chemical_burn_immunity",	// eHitTypeChemicalBurn=2
	"radiation_immunity",		// eHitTypeRadiation=3
	"telepatic_immunity",		// eHitTypeTelepatic=4
	"wound_immunity",			// eHitTypeWound=5
	"fire_wound_immunity",		// eHitTypeFireWound=6
	"strike_immunity",			// eHitTypeStrike=7
	"explosion_immunity",		// eHitTypeExplosion=8
	"",							// eHitTypeWound_2=9
	"",							// eHitTypeLightBurn=10
};

LPCSTR af_restore_section_names[] = // ALife::EConditionRestoreType
{
	"health_restore_speed",			// eHealthRestoreSpeed=0
	"satiety_restore_speed",		// eSatietyRestoreSpeed=1
	"thirst_restore_speed",			// eThirstRestoreSpeed=3
	"radiation_restore_speed",		// eRadiationRestoreSpeed=2
	"power_restore_speed",			// ePowerRestoreSpeed=4
	"bleeding_restore_speed",		// eBleedingRestoreSpeed=5
	"psy_health_restore_speed",		// ePsyHealthRestoreSpeed=6
	"sleepeness_restore_speed",		// eSleepenessRestoreSpeed=7
	"intoxication_restore_speed",	// eIntoxicationRestoreSpeed=8
	"alcoholism_restore_speed",		// eAlcoholismRestoreSpeed=9
	"hangover_restore_speed",		// eHangoverRestoreSpeed=10
	"narcotism_restore_speed",		// eNarcotismRestoreSpeed=11
	"withdrawal_restore_speed",		// eWithDrawalRestoreSpeed=12
	"frostbite_restore_speed",		// eFrostbiteRestoreSpeed=13
};

LPCSTR af_immunity_caption[] =  // ALife::EHitType
{
	"ui_inv_outfit_burn_protection",			// eHitTypeBurn=0
	"ui_inv_outfit_shock_protection",			// eHitTypeShock=1
	"ui_inv_outfit_chemical_burn_protection",	// eHitTypeChemicalBurn=3
	"ui_inv_outfit_radiation_protection",		// eHitTypeRadiation=4
	"ui_inv_outfit_telepatic_protection",		// eHitTypeTelepatic=5
	"ui_inv_outfit_wound_protection",			// eHitTypeWound=6
	"ui_inv_outfit_fire_wound_protection",		// eHitTypeFireWound=8
	"ui_inv_outfit_strike_protection",			// eHitTypeStrike=7
	"ui_inv_outfit_explosion_protection",		// eHitTypeExplosion=8
	"",											// eHitTypeWound_2=9
	"",											// eHitTypeLightBurn=10
};

LPCSTR af_restore_caption[] =  // ALife::EConditionRestoreType
{
	"ui_inv_health",
	"ui_inv_satiety",
	"ui_inv_thirst",
	"ui_inv_radiation",
	"ui_inv_power",
	"ui_inv_bleeding",
	"ui_inv_psy_health",
	"ui_inv_sleepeness",
	"ui_inv_intoxication",
	"ui_inv_alcoholism",
	"ui_inv_hangover",
	"ui_inv_narcotism",
	"ui_inv_withdrawal",
	"ui_inv_frostbite",
};

void CUIArtefactParams::InitFromXml( CUIXml& xml )
{
	LPCSTR base	= "af_params";

	XML_NODE* stored_root = xml.GetLocalRoot();
	XML_NODE* base_node   = xml.NavigateToNode( base, 0 );
	if ( !base_node )
	{
		return;
	}
	CUIXmlInit::InitWindow( xml, base, 0, this );
	xml.SetLocalRoot( base_node );
	
	m_Prop_line = xr_new<CUIStatic>();
	AttachChild( m_Prop_line );
	m_Prop_line->SetAutoDelete( false );	
	CUIXmlInit::InitStatic( xml, "prop_line", 0, m_Prop_line );

	for ( u32 i = 0; i < ALife::eHitTypeMax; ++i )
	{
		if (i >= ALife::eHitTypeWound_2)
			continue;
		m_immunity_item[i] = xr_new<UIArtefactParamItem>();
		m_immunity_item[i]->Init( xml, af_immunity_section_names[i] );
		m_immunity_item[i]->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate(af_immunity_caption[i]).c_str();
		m_immunity_item[i]->SetCaption( name );

		xml.SetLocalRoot( base_node );
	}
	
	for ( u32 i = 0; i < ALife::eRestoreTypeMax; ++i )
	{
		m_restore_item[i] = xr_new<UIArtefactParamItem>();
		m_restore_item[i]->Init( xml, af_restore_section_names[i] );
		m_restore_item[i]->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate(af_restore_caption[i]).c_str();
		m_restore_item[i]->SetCaption( name );

		xml.SetLocalRoot( base_node );
	}

	m_fWalkAccel = xr_new<UIArtefactParamItem>();
	m_fWalkAccel->Init(xml, "walk_accel");
	m_fWalkAccel->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_walk_accel").c_str();
	m_fWalkAccel->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fJumpSpeed = xr_new<UIArtefactParamItem>();
	m_fJumpSpeed->Init(xml, "jump_speed");
	m_fJumpSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_jump_speed").c_str();
	m_fJumpSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);
	
	m_additional_weight = xr_new<UIArtefactParamItem>();
	m_additional_weight->Init(xml, "additional_weight");
	m_additional_weight->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_weight").c_str();
	m_additional_weight->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_iArtefactRank = xr_new<UIArtefactParamItem>();
	m_iArtefactRank->Init(xml, "artefact_rank");
	m_iArtefactRank->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_af_rank").c_str();
	m_iArtefactRank->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fChargeLevel = xr_new<UIArtefactParamItem>();
	m_fChargeLevel->Init(xml, "artefact_charge_level");
	m_fChargeLevel->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_artefact_charge").c_str();
	m_fChargeLevel->SetCaption(name);
	xml.SetLocalRoot(base_node);

	xml.SetLocalRoot( stored_root );
}

bool CUIArtefactParams::Check(const shared_str& af_section)
{
	return !!pSettings->line_exist(af_section, "af_actor_properties");
}

bool CUIArtefactParams::CheckDescrInfoPortions(const shared_str& af_section)
{
	if (pSettings->line_exist(af_section, "af_description_infoportion"))
	{
		shared_str ArtifactDescrInfo = pSettings->r_string(af_section, "af_description_infoportion");
		if (Actor()->HasInfo(ArtifactDescrInfo))
			return true;
		else
			return false;
	}
	else
		return true;
}

void CUIArtefactParams::SetInfo(CInventoryItem& pInvItem)
{
	DetachAll();
	AttachChild( m_Prop_line );

	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if ( !actor )
	{
		return;
	}

	float val = 0.0f, max_val = 1.0f;
	Fvector2 pos;
	float h = m_Prop_line->GetWndPos().y+m_Prop_line->GetWndSize().y;
	const shared_str& af_section = pInvItem.object().cNameSect();
	CArtefact* artefact = pInvItem.object().cast_artefact();

	for ( u32 i = 0; i < ALife::eHitTypeMax; ++i )
	{
		if (i >= ALife::eHitTypeWound_2)
			continue;
		val = artefact->m_HitTypeProtection[(ALife::EHitType)i];
		if ( fis_zero(val) )
		{
			continue;
		}
		max_val = actor->conditions().GetZoneMaxPower((ALife::EHitType)i);
		val /= max_val;
		m_immunity_item[i]->SetValue(val, 2);

		pos.set( m_immunity_item[i]->GetWndPos() );
		pos.y = h;
		m_immunity_item[i]->SetWndPos( pos );

		h += m_immunity_item[i]->GetWndSize().y;
		AttachChild( m_immunity_item[i] );
	}
	for ( u32 i = 0; i < ALife::eRestoreTypeMax; ++i )
	{
		val = artefact->GetRestoreByType((ALife::EConditionRestoreType)i);
		if ( fis_zero(val) )
		{
			continue;
		}
		if (i == ALife::eRadiationRestoreSpeed || i == ALife::eIntoxicationRestoreSpeed || i == ALife::eSleepenessRestoreSpeed || i == ALife::eAlcoholismRestoreSpeed || i == ALife::eFrostbiteRestoreSpeed)
			m_restore_item[i]->SetValue(val, 1);
		else
			m_restore_item[i]->SetValue(val, 2);

		pos.set(m_restore_item[i]->GetWndPos() );
		pos.y = h;
		m_restore_item[i]->SetWndPos( pos );

		h += m_restore_item[i]->GetWndSize().y;
		AttachChild(m_restore_item[i] );
	}

	if (artefact)
	{
		val = artefact->m_additional_weight;
		if (!fis_zero(val))
		{
			m_additional_weight->SetValue(val, 2);

			pos.set(m_additional_weight->GetWndPos());
			pos.y = h;
			m_additional_weight->SetWndPos(pos);

			h += m_additional_weight->GetWndSize().y;
			AttachChild(m_additional_weight);
		}

		val = artefact->m_fWalkAccel;
		if (!fis_zero(val) && val > 1.0f)
		{
			m_fWalkAccel->SetValue(val, 2);

			pos.set(m_fWalkAccel->GetWndPos());
			pos.y = h;
			m_fWalkAccel->SetWndPos(pos);

			h += m_fWalkAccel->GetWndSize().y;
			AttachChild(m_fWalkAccel);
		}

		val = artefact->m_fJumpSpeed;
		if (!fis_zero(val) && val > 1.0f)
		{
			m_fJumpSpeed->SetValue(val, 2);

			pos.set(m_fJumpSpeed->GetWndPos());
			pos.y = h;
			m_fJumpSpeed->SetWndPos(pos);

			h += m_fJumpSpeed->GetWndSize().y;
			AttachChild(m_fJumpSpeed);
		}

		val = (float)artefact->m_iAfRank;
		if (!fis_zero(val) && GameConstants::GetAfRanks())
		{
			m_iArtefactRank->SetValue(val);

			pos.set(m_iArtefactRank->GetWndPos());
			pos.y = h;
			m_iArtefactRank->SetWndPos(pos);

			h += m_iArtefactRank->GetWndSize().y;
			AttachChild(m_iArtefactRank);
		}

		val = artefact->m_fChargeLevel;
		if (!fis_zero(val) && GameConstants::GetArtefactsDegradation())
		{
			m_fChargeLevel->SetValue(val);

			pos.set(m_fChargeLevel->GetWndPos());
			pos.y = h;
			m_fChargeLevel->SetWndPos(pos);

			h += m_fChargeLevel->GetWndSize().y;
			AttachChild(m_fChargeLevel);
		}
	}

	SetHeight( h );
}

/// ----------------------------------------------------------------

UIArtefactParamItem::UIArtefactParamItem()
{
	m_caption   = NULL;
	m_value     = NULL;
	m_magnitude = 1.0f;
	m_sign_inverse = false;
	m_show_sign = false;
	
	m_unit_str._set( "" );
	m_texture._set("");
}

UIArtefactParamItem::~UIArtefactParamItem()
{
}

void UIArtefactParamItem::Init( CUIXml& xml, LPCSTR section )
{
	CUIXmlInit::InitWindow( xml, section, 0, this );
	xml.SetLocalRoot( xml.NavigateToNode( section ) );

	m_caption   = UIHelper::CreateStatic( xml, "caption", this );
	m_value     = UIHelper::CreateTextWnd( xml, "value",   this );
	m_magnitude = xml.ReadAttribFlt( "value", 0, "magnitude", 1.0f );
	m_sign_inverse = (xml.ReadAttribInt( "value", 0, "sign_inverse", 0 ) == 1);
	m_show_sign = (xml.ReadAttribInt("value", 0, "show_sign", 1) == 1);

	LPCSTR unit_str = xml.ReadAttrib( "value", 0, "unit_str", "" );
	m_unit_str._set( CStringTable().translate( unit_str ) );

	if (xml.NavigateToNode("caption:texture", 0))
	{
		use_color = xml.ReadAttribInt("caption:texture", 0, "use_color", 0);
		clr_invert = xml.ReadAttribInt("caption:texture", 0, "clr_invert", 0);
		clr_dynamic = xml.ReadAttribInt("caption:texture", 0, "clr_dynamic", 0);
		LPCSTR texture = xml.Read("caption:texture", 0, "");
		m_texture._set(texture);
	}

	Ivector4 red = GameConstants::GetRedColor();
	Ivector4 green = GameConstants::GetGreenColor();
	Ivector4 neutral = GameConstants::GetNeutralColor();

	if (xml.NavigateToNode("caption:min_color", 0))
		m_negative_color = CUIXmlInit::GetColor(xml, "caption:min_color", 0, color_rgba(red.x, red.y, red.z, red.w));
	else
		m_negative_color = color_rgba(red.x, red.y, red.z, red.w);

	if (xml.NavigateToNode("caption:middle_color", 0))
		m_neutral_color = CUIXmlInit::GetColor(xml, "caption:middle_color", 0, color_rgba(neutral.x, neutral.y, neutral.z, neutral.w));
	else
		m_neutral_color = color_rgba(neutral.x, neutral.y, neutral.z, neutral.w);

	if (xml.NavigateToNode("caption:max_color", 0))
		m_positive_color = CUIXmlInit::GetColor(xml, "caption:max_color", 0, color_rgba(green.x, green.y, green.z, green.w));
	else
		m_positive_color = color_rgba(green.x, green.y, green.z, green.w);
}

void UIArtefactParamItem::SetCaption( LPCSTR name )
{
	m_caption->TextItemControl()->SetText( name );
}

void UIArtefactParamItem::SetValue(float value, int vle)
{
	value *= m_magnitude;
	string32	buf;

	if (m_show_sign)
		xr_sprintf(buf, "%+.0f", value);
	else
		xr_sprintf(buf, "%.0f", value);
	
	LPSTR		str;
	if ( m_unit_str.size() )
	{
		STRCONCAT( str, buf, " ", m_unit_str.c_str() );
	}
	else // = ""
	{
		STRCONCAT( str, buf );
	}
	m_value->SetText( str );

	bool is_positive = (value >= 0.0f);
	Fcolor current{}, negative{}, middle{}, positive{};

	value /= m_magnitude;
	clamp(value, 0.01f, 1.0f);

	if (GameConstants::GetColorizeValues())
	{
		if (vle == 0)
		{
			m_value->SetTextColor(m_neutral_color);
		}
		else if (vle == 1)
		{
			if (is_positive)
				current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), value);
			else
				current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), value);
		}
		else if (vle == 2)
		{
			if (is_positive)
				current.lerp(negative.set(m_positive_color), middle.set(m_neutral_color), positive.set(m_negative_color), 0.25f);
			else
				current.lerp(negative.set(m_negative_color), middle.set(m_neutral_color), positive.set(m_positive_color), 0.25f);
		}

		if (vle == 1 || vle == 2)
			m_value->SetTextColor(current.get());
	}
	else
		m_value->SetTextColor(color_rgba(170, 170, 170, 255));

	m_caption->InitTexture(m_texture.c_str());

	if (GameConstants::GetColorizeValues() || use_color)
	{
		if (clr_dynamic)
		{
			if (vle >= 2 || !clr_invert)
			{
				if (is_positive)
					current.lerp(negative.set(m_negative_color), positive.set(m_positive_color), value);
				else
					current.lerp(negative.set(m_positive_color), positive.set(m_negative_color), value);
			}
			else
			{
				if (is_positive)
					current.lerp(negative.set(m_positive_color), positive.set(m_negative_color), value);
				else
					current.lerp(negative.set(m_negative_color), positive.set(m_positive_color), value);
			}

			m_caption->SetTextureColor(current.get());
		}
		else
			m_caption->SetTextureColor(m_neutral_color);
	}
}
