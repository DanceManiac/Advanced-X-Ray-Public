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

u32 const red_clr   = color_argb( 250, 210, 50,  50 );
u32 const green_clr = color_argb( 250, 50,  210, 50 );

CUIArtefactParams::CUIArtefactParams()
{
	for ( u32 i = 0; i < ALife::infl_max_count; ++i )
	{
		m_immunity_item[i] = NULL;
	}

	m_fHealthRestoreSpeed = NULL;
	m_fRadiationRestoreSpeed = NULL;
	m_fSatietyRestoreSpeed = NULL;
	m_fPowerRestoreSpeed = NULL;
	m_fBleedingRestoreSpeed = NULL;
	m_fThirstRestoreSpeed = NULL;
	m_fIntoxicationRestoreSpeed = NULL;
	m_fSleepenessRestoreSpeed = NULL;
	m_fAlcoholismRestoreSpeed = NULL;
	m_fPsyHealthRestoreSpeed = NULL;
	m_fWalkAccel = NULL;
	m_fJumpSpeed = NULL;
	m_additional_weight = NULL;
	m_iArtefactRank = NULL;
	m_fChargeLevel = NULL;
}

CUIArtefactParams::~CUIArtefactParams()
{
	delete_data(m_immunity_item);
	xr_delete(m_fHealthRestoreSpeed);
	xr_delete(m_fRadiationRestoreSpeed);
	xr_delete(m_fSatietyRestoreSpeed);
	xr_delete(m_fPowerRestoreSpeed);
	xr_delete(m_fBleedingRestoreSpeed);
	xr_delete(m_fThirstRestoreSpeed);
	xr_delete(m_fIntoxicationRestoreSpeed);
	xr_delete(m_fSleepenessRestoreSpeed);
	xr_delete(m_fAlcoholismRestoreSpeed);
	xr_delete(m_fPsyHealthRestoreSpeed);
	xr_delete(m_fWalkAccel);
	xr_delete(m_fJumpSpeed);
	xr_delete(m_additional_weight);
	xr_delete(m_iArtefactRank);
	xr_delete(m_fChargeLevel);
}

LPCSTR af_immunity_section_names[] = // ALife::EInfluenceType
{
	"radiation_immunity",		// infl_rad=0
	"burn_immunity",			// infl_fire=1
	"chemical_burn_immunity",	// infl_acid=2
	"telepatic_immunity",		// infl_psi=3
	"shock_immunity",			// infl_electra=4
	"strike_immunity",			// infl_strike=5
	"wound_immunity",			// infl_wound=6
	"explosion_immunity",		// infl_explossion=7
	"fire_wound_immunity",		// infl_fire_wound=8
};

LPCSTR af_restore_section_names[] = // ALife::EConditionRestoreType
{
	"health_restore_speed",			// eHealthRestoreSpeed=0
	"radiation_restore_speed",		// eRadiationRestoreSpeed=1
	"satiety_restore_speed",		// eSatietyRestoreSpeed=2
	"power_restore_speed",			// ePowerRestoreSpeed=3
	"bleeding_restore_speed",		// eBleedingRestoreSpeed=4
	"thirst_restore_speed",			// eThirstRestoreSpeed=5
	"intoxication_restore_speed",	// eIntoxicationRestoreSpeed=6
	"sleepeness_restore_speed",		// eSleepenessRestoreSpeed=7
	"alcoholism_restore_speed",		// eAlcoholismRestoreSpeed=8
	"psy_health_restore_speed",		// ePsyHealthRestoreSpeed=9
};

LPCSTR af_immunity_caption[] =  // ALife::EInfluenceType
{
	"ui_inv_outfit_radiation_protection",		// "(radiation_imm)",
	"ui_inv_outfit_burn_protection",			// "(burn_imm)",
	"ui_inv_outfit_chemical_burn_protection",	// "(chemical_burn_imm)",
	"ui_inv_outfit_telepatic_protection",		// "(telepatic_imm)",
	"ui_inv_outfit_shock_protection",			// "(shock_imm)",
	"ui_inv_outfit_strike_protection",			// "(strike_imm)",
	"ui_inv_outfit_wound_protection",			// "(wound_imm)",
	"ui_inv_outfit_explosion_protection",		// "(explosion_imm)",
	"ui_inv_outfit_fire_wound_protection",		// "(fire_wound_imm)",
};

LPCSTR af_restore_caption[] =  // ALife::EConditionRestoreType
{
	"ui_inv_health",
	"ui_inv_radiation",
	"ui_inv_satiety",
	"ui_inv_power",
	"ui_inv_bleeding",
	"ui_inv_thirst",
	"ui_inv_intoxication",
	"ui_inv_sleepeness",
	"ui_inv_alcoholism",
	"ui_inv_psy_health",
};

/*
LPCSTR af_actor_param_names[]=
{
	"satiety_health_v",
	"radiation_v",
	"satiety_v",
	"satiety_power_v",
	"wound_incarnation_v",
};
*/

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
	
	for ( u32 i = 0; i < ALife::infl_max_count; ++i )
	{
		m_immunity_item[i] = xr_new<UIArtefactParamItem>();
		m_immunity_item[i]->Init( xml, af_immunity_section_names[i] );
		m_immunity_item[i]->SetAutoDelete(false);

		LPCSTR name = CStringTable().translate(af_immunity_caption[i]).c_str();
		m_immunity_item[i]->SetCaption( name );

		xml.SetLocalRoot( base_node );
	}

	m_fHealthRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fHealthRestoreSpeed->Init(xml, "health_restore_speed");
	m_fHealthRestoreSpeed->SetAutoDelete(false);
	LPCSTR name = CStringTable().translate("ui_inv_health").c_str();
	m_fHealthRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fRadiationRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fRadiationRestoreSpeed->Init(xml, "radiation_restore_speed");
	m_fRadiationRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_radiation").c_str();
	m_fRadiationRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fSatietyRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fSatietyRestoreSpeed->Init(xml, "satiety_restore_speed");
	m_fSatietyRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_satiety").c_str();
	m_fSatietyRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fPowerRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fPowerRestoreSpeed->Init(xml, "power_restore_speed");
	m_fPowerRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_power").c_str();
	m_fPowerRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fBleedingRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fBleedingRestoreSpeed->Init(xml, "bleeding_restore_speed");
	m_fBleedingRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_bleeding").c_str();
	m_fBleedingRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fThirstRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fThirstRestoreSpeed->Init(xml, "thirst_restore_speed");
	m_fThirstRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_thirst").c_str();
	m_fThirstRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fIntoxicationRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fIntoxicationRestoreSpeed->Init(xml, "intoxication_restore_speed");
	m_fIntoxicationRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_intoxication").c_str();
	m_fIntoxicationRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fSleepenessRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fSleepenessRestoreSpeed->Init(xml, "sleepeness_restore_speed");
	m_fSleepenessRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_sleepeness").c_str();
	m_fSleepenessRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fAlcoholismRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fAlcoholismRestoreSpeed->Init(xml, "alcoholism_restore_speed");
	m_fAlcoholismRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_alcoholism").c_str();
	m_fAlcoholismRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fPsyHealthRestoreSpeed = xr_new<UIArtefactParamItem>();
	m_fPsyHealthRestoreSpeed->Init(xml, "psy_health_restore_speed");
	m_fPsyHealthRestoreSpeed->SetAutoDelete(false);
	name = CStringTable().translate("ui_inv_psy_health").c_str();
	m_fPsyHealthRestoreSpeed->SetCaption(name);
	xml.SetLocalRoot(base_node);

	m_fWalkAccel = xr_new<UIArtefactParamItem>();
	m_fWalkAccel->Init(xml, "walk_accel");
	m_fWalkAccel->SetAutoDelete(false);
	name = CStringTable().translate("ui_walk_accel").c_str();
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
	xml.SetLocalRoot(stored_root);

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
	float h = 0.0f;

	CActor* actor = smart_cast<CActor*>( Level().CurrentViewEntity() );
	if ( !actor )
	{
		return;
	}

	float val = 0.0f, max_val = 1.0f;
	Fvector2 pos;
	const shared_str& af_section = pInvItem.object().cNameSect();
	CArtefact* artefact = pInvItem.object().cast_artefact();

	for ( u32 i = 0; i < ALife::infl_max_count; ++i )
	{
		val = artefact->m_HitTypeProtection[(ALife::EInfluenceType)i];
		if ( fis_zero(val) )
		{
			continue;
		}
		max_val = actor->conditions().GetZoneMaxPower( (ALife::EInfluenceType)i );
		val /= max_val;

		m_immunity_item[i]->SetValue(val, 2);

		pos.set( m_immunity_item[i]->GetWndPos() );
		pos.y = h;
		m_immunity_item[i]->SetWndPos( pos );

		h += m_immunity_item[i]->GetWndSize().y;
		AttachChild( m_immunity_item[i] );
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

		val = artefact->m_fHealthRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fHealthRestoreSpeed->SetValue(val, 2);

			pos.set(m_fHealthRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fHealthRestoreSpeed->SetWndPos(pos);

			h += m_fHealthRestoreSpeed->GetWndSize().y;
			AttachChild(m_fHealthRestoreSpeed);
		}

		val = artefact->m_fRadiationRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fRadiationRestoreSpeed->SetValue(val, 1);

			pos.set(m_fRadiationRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fRadiationRestoreSpeed->SetWndPos(pos);

			h += m_fRadiationRestoreSpeed->GetWndSize().y;
			AttachChild(m_fRadiationRestoreSpeed);
		}

		val = artefact->m_fSatietyRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fSatietyRestoreSpeed->SetValue(val, 2);

			pos.set(m_fSatietyRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fSatietyRestoreSpeed->SetWndPos(pos);

			h += m_fSatietyRestoreSpeed->GetWndSize().y;
			AttachChild(m_fSatietyRestoreSpeed);
		}

		val = artefact->m_fPowerRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fPowerRestoreSpeed->SetValue(val, 2);

			pos.set(m_fPowerRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fPowerRestoreSpeed->SetWndPos(pos);

			h += m_fPowerRestoreSpeed->GetWndSize().y;
			AttachChild(m_fPowerRestoreSpeed);
		}

		val = artefact->m_fBleedingRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fBleedingRestoreSpeed->SetValue(val, 2);

			pos.set(m_fBleedingRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fBleedingRestoreSpeed->SetWndPos(pos);

			h += m_fBleedingRestoreSpeed->GetWndSize().y;
			AttachChild(m_fBleedingRestoreSpeed);
		}

		val = artefact->m_fThirstRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fThirstRestoreSpeed->SetValue(val, 2);

			pos.set(m_fThirstRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fThirstRestoreSpeed->SetWndPos(pos);

			h += m_fThirstRestoreSpeed->GetWndSize().y;
			AttachChild(m_fThirstRestoreSpeed);
		}

		val = artefact->m_fIntoxicationRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fIntoxicationRestoreSpeed->SetValue(val, 1);

			pos.set(m_fIntoxicationRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fIntoxicationRestoreSpeed->SetWndPos(pos);

			h += m_fIntoxicationRestoreSpeed->GetWndSize().y;
			AttachChild(m_fIntoxicationRestoreSpeed);
		}

		val = artefact->m_fSleepenessRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fSleepenessRestoreSpeed->SetValue(val, 1);

			pos.set(m_fSleepenessRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fSleepenessRestoreSpeed->SetWndPos(pos);

			h += m_fSleepenessRestoreSpeed->GetWndSize().y;
			AttachChild(m_fSleepenessRestoreSpeed);
		}

		val = artefact->m_fAlcoholismRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fAlcoholismRestoreSpeed->SetValue(val, 1);

			pos.set(m_fAlcoholismRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fAlcoholismRestoreSpeed->SetWndPos(pos);

			h += m_fAlcoholismRestoreSpeed->GetWndSize().y;
			AttachChild(m_fAlcoholismRestoreSpeed);
		}

		val = artefact->m_fPsyHealthRestoreSpeed;
		if (!fis_zero(val))
		{
			m_fPsyHealthRestoreSpeed->SetValue(val, 2);

			pos.set(m_fPsyHealthRestoreSpeed->GetWndPos());
			pos.y = h;
			m_fPsyHealthRestoreSpeed->SetWndPos(pos);

			h += m_fPsyHealthRestoreSpeed->GetWndSize().y;
			AttachChild(m_fPsyHealthRestoreSpeed);
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

		val = artefact->m_iAfRank;
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
	m_value     = UIHelper::CreateStatic( xml, "value",   this );
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

	Fvector4 red = GameConstants::GetRedColor();
	Fvector4 green = GameConstants::GetGreenColor();
	Fvector4 neutral = GameConstants::GetNeutralColor();

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
	m_caption->SetText( name );
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
