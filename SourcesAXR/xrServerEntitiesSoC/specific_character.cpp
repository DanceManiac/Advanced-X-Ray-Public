#include "stdafx.h"
#include "specific_character.h"

#ifdef  XRGAME_EXPORTS
#include "PhraseDialog.h"
#include "string_table.h"

#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING

#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

SSpecificCharacterData::SSpecificCharacterData()
{
	m_sGameName.clear		();
	m_sBioText				= NULL;
	m_sVisual.clear			();
	m_sSupplySpawn.clear	();
	m_sNpcConfigSect.clear	();


	m_StartDialog			= NULL;
	m_ActorDialogs.clear	(); 

	m_Rank					= NO_RANK;
	m_Reputation			= NO_REPUTATION;

	m_bNoRandom				= false;
	m_bDefaultForCommunity	= false;
	m_fPanic_threshold		= 0.0f;
	m_fHitProbabilityFactor	= 1.f;
	m_crouch_type			= 0;

	first_visual			= -1;
	last_visual				= -1;
}

SSpecificCharacterData::~SSpecificCharacterData()
{
}

#endif

CSpecificCharacter::CSpecificCharacter()
{
	m_OwnId = NULL;
}


CSpecificCharacter::~CSpecificCharacter()
{
}


void CSpecificCharacter::InitXmlIdToIndex()
{
	if(!id_to_index::tag_name)
		id_to_index::tag_name = "specific_character";
	if(!id_to_index::file_str)
		id_to_index::file_str = pSettings->r_string("profiles", "specific_characters_files");
}


void CSpecificCharacter::Load(shared_str id)
{
	R_ASSERT(id.size());
	m_OwnId = id;
	inherited_shared::load_shared(m_OwnId, NULL);
}


void CSpecificCharacter::load_shared	(LPCSTR)
{
#if 0
	CTimer			timer;
	timer.Start		();
#endif
	const ITEM_DATA& item_data = *id_to_index::GetById(m_OwnId);

	CUIXml*		pXML = item_data._xml;

	pXML->SetLocalRoot		(pXML->GetRoot());


	XML_NODE* item_node = pXML->NavigateToNode(id_to_index::tag_name, item_data.pos_in_file);
	R_ASSERT3(item_node, "specific_character id=", *item_data.id);

	pXML->SetLocalRoot(item_node);



	int norandom = pXML->ReadAttribInt(item_node, "no_random", 0);
	if (1 == norandom) 
		data()->m_bNoRandom = true;
	else
		data()->m_bNoRandom = false;

	int team_default = pXML->ReadAttribInt(item_node, "team_default", 0);
	if (1 == team_default) 
		data()->m_bDefaultForCommunity = true;
	else
		data()->m_bDefaultForCommunity = false;

	R_ASSERT3(!(data()->m_bNoRandom && data()->m_bDefaultForCommunity), 
		"cannot set 'no_random' and 'team_default' flags simultaneously, profile id", *shared_str(item_data.id));
	
#ifdef  XRGAME_EXPORTS

	LPCSTR start_dialog = pXML->Read("start_dialog", 0, NULL);
	if(start_dialog)
	{
		data()->m_StartDialog	= start_dialog;
	}
	else
		data()->m_StartDialog	= NULL;

	int dialogs_num = pXML->GetNodesNum(pXML->GetLocalRoot(), "actor_dialog");
	data()->m_ActorDialogs.clear();
	for(int i=0; i<dialogs_num; ++i)
	{
		shared_str dialog_name = pXML->Read(pXML->GetLocalRoot(), "actor_dialog", i, "");
		data()->m_ActorDialogs.push_back(dialog_name);
	}	

	//игровое имя персонажа
	data()->m_sGameName		= pXML->Read("name", 0, "");
	data()->m_sBioText		= CStringTable().translate(pXML->Read("bio", 0, ""));


	data()->m_fPanic_threshold		= pXML->ReadFlt("panic_threshold",0,0.f);
	data()->m_fHitProbabilityFactor	= pXML->ReadFlt("hit_probability_factor",0,1.f);
	data()->m_crouch_type			= pXML->ReadInt("crouch_type",0,0);

	data()->m_critical_wound_weights= pXML->Read("critical_wound_weights", 0, "1");

#endif

	data()->m_sVisual		= pXML->Read("visual", 0, "");
	
	data()->m_bForceDisabledRandomIcons = pXML->ReadAttribInt("icon", 0, "force_disabled_random", 0) == 1;

	if (data()->m_sVisual.back() != '_' || data()->m_bForceDisabledRandomIcons)
		data()->m_icon_name = pXML->Read("icon", 0, "ui_npc_u_barman");

#ifdef  XRGAME_EXPORTS
	data()->m_sSupplySpawn	= pXML->Read("supplies", 0, "");
	
	if(!data()->m_sSupplySpawn.empty())
	{
		xr_string &str = data()->m_sSupplySpawn;
		xr_string::size_type pos = str.find("\\n");

		while (xr_string::npos != pos)
		{
			str.replace(pos, 2, "\n");
			pos = str.find("\\n", pos + 1);
		}
	}

	data()->m_sNpcConfigSect		= pXML->Read("npc_config", 0, "");
	data()->m_sound_voice_prefix	= pXML->Read("snd_config", 0, "");

	data()->m_terrain_sect			= pXML->Read("terrain_sect", 0, "");

#endif

	data()->m_Classes.clear			();
	int classes_num					= pXML->GetNodesNum (pXML->GetLocalRoot(), "class");
	for(int i=0; i<classes_num; i++)
	{
		LPCSTR char_class			= pXML->Read	("class", 0, "");
		if(char_class)
		{
			char* buf_str			= xr_strdup(char_class);
			xr_strlwr				(buf_str);
			data()->m_Classes.push_back(buf_str);
			xr_free					(buf_str);
		}
	}


#ifdef  XRGAME_EXPORTS

	LPCSTR team = pXML->Read("community", 0, NULL);
	R_ASSERT3(team != NULL, "'community' field not fulfiled for specific character", *m_OwnId);
	
	char* buf_str = xr_strdup(team);
	xr_strlwr(buf_str);
	data()->m_Community.set(buf_str);
	xr_free(buf_str);
	
	if(data()->m_Community.index() == NO_COMMUNITY_INDEX)
		Debug.fatal(DEBUG_INFO,"wrong 'community' '%s' in specific character %s ", team, *m_OwnId);

	data()->m_Rank			= pXML->ReadInt("rank", 0, NO_RANK);
	R_ASSERT3(data()->m_Rank != NO_RANK, "'rank' field not fulfiled for specific character", *m_OwnId);
	data()->m_Reputation	= pXML->ReadInt("reputation", 0, NO_REPUTATION);
	R_ASSERT3(data()->m_Reputation != NO_REPUTATION, "'reputation' field not fulfiled for specific character", *m_OwnId);

	if(pXML->NavigateToNode(pXML->GetLocalRoot(), "money", 0))
	{
		MoneyDef().min_money	= pXML->ReadAttribInt("money", 0, "min");
		MoneyDef().max_money	= pXML->ReadAttribInt("money", 0, "max");
		MoneyDef().inf_money	= !!pXML->ReadAttribInt("money", 0, "infinitive");
		MoneyDef().max_money	= _max(MoneyDef().max_money, MoneyDef().min_money); // :)

	}else{
		MoneyDef().min_money	= 0;
		MoneyDef().max_money	= 0;
		MoneyDef().inf_money	= false;
	}

#endif

#if 0
	Msg			("CSpecificCharacter::load_shared() takes %f milliseconds",timer.GetElapsed_sec()*1000.f);
#endif
}


#ifdef  XRGAME_EXPORTS

LPCSTR CSpecificCharacter::Name() const 
{
	return	data()->m_sGameName.c_str();
}

shared_str CSpecificCharacter::Bio() const 
{
	return	data()->m_sBioText;
}

const CHARACTER_COMMUNITY& CSpecificCharacter::Community() const 
{
	return	data()->m_Community;
}

LPCSTR CSpecificCharacter::SupplySpawn	() const 
{
	return data()->m_sSupplySpawn.c_str();
}

LPCSTR CSpecificCharacter::NpcConfigSect () const 
{
	return data()->m_sNpcConfigSect.c_str();
}

LPCSTR CSpecificCharacter::sound_voice_prefix () const 
{
	return data()->m_sound_voice_prefix.c_str();
}

float CSpecificCharacter::panic_threshold () const 
{
	return data()->m_fPanic_threshold;
}

float CSpecificCharacter::hit_probability_factor () const 
{
	return data()->m_fHitProbabilityFactor;
}

int CSpecificCharacter::crouch_type	() const 
{
	return data()->m_crouch_type;
}

LPCSTR CSpecificCharacter::critical_wound_weights () const 
{
	return data()->m_critical_wound_weights.c_str();
}

#endif

shared_str CSpecificCharacter::terrain_sect		() const
{
	return	data()->m_terrain_sect;
}

CHARACTER_RANK_VALUE CSpecificCharacter::Rank() const 
{
	return	data()->m_Rank;
}

CHARACTER_REPUTATION_VALUE CSpecificCharacter::Reputation	() const 
{
	return data()->m_Reputation;
}

LPCSTR CSpecificCharacter::Visual()
{
	xr_string visual_name = data()->m_sVisual.c_str();

	int rnd_vis{};

	if (visual_name.back() == '_')
	{
		SetRandomRange();

		if ((data()->last_visual != data()->first_visual) && (data()->first_visual < data()->last_visual))
			rnd_vis = ::Random.randI(data()->first_visual, data()->last_visual);
		else
			rnd_vis = data()->last_visual;

		xr_string visual_randomized = visual_name;
		visual_randomized += std::to_string(rnd_vis).c_str();

		// Icon
		if (!data()->m_bForceDisabledRandomIcons)
		{
			xr_string randomized_icon = "ui_npc_";
			size_t lastBackslashPos = visual_randomized.find_last_of('\\');

			xr_string result = visual_randomized.substr(lastBackslashPos + 1);
			randomized_icon += result.c_str();

			data()->m_icon_name = randomized_icon.c_str();
		}

		return visual_randomized.c_str();
	}
	else if (visual_name.back() == '*')
	{
		std::string visual_name = data()->m_sVisual.c_str();

		string_path full_mask;
		xr_strcpy(full_mask, data()->m_sVisual.c_str());
		xr_strcat(full_mask, "*");

		FS_FileSet fset;
		FS.file_list(fset, "$game_meshes$", FS_ListFiles, full_mask);

		if (fset.empty())
		{
			Msg("[CSpecificCharacter::Visual]: File list is empty! Check visuals folder!");
			return data()->m_sVisual.c_str();
		}

		rnd_vis = ::Random.randI(0, fset.size());

		FS_FileSetIt it = fset.begin();
		std::advance(it, rnd_vis);

		// Icon
		if (!data()->m_bForceDisabledRandomIcons)
		{
			xr_string randomized_icon = "ui_npc_";
			size_t lastBackslashPos = it->name.find_last_of('\\');

			xr_string result = it->name.substr(lastBackslashPos + 1);
			randomized_icon += result.c_str();

			data()->m_icon_name = randomized_icon.c_str();
		}

		return it->name.c_str();
	}

	return data()->m_sVisual.c_str();
}

void CSpecificCharacter::SetRandomRange()
{
	int min_num = 1;
	int max_num = 1;

	std::string visual_name = data()->m_sVisual.c_str();

	string_path full_mask;
	xr_strcpy(full_mask, data()->m_sVisual.c_str());
	xr_strcat(full_mask, "*");

	FS_FileSet fset;
	FS.file_list(fset, "$game_meshes$", FS_ListFiles, full_mask);
	FS_FileSetIt fit = fset.begin();
	const FS_FileSetIt fit_e = fset.end();

	bool filesFound = false;

	for (; fit != fit_e; ++fit)
	{
		std::string name = fit->name.c_str();

		if (name.find(visual_name) == 0)
		{
			try
			{
				int num = std::stoi(name.substr(visual_name.length()));

				if (!filesFound)
				{
					min_num = num;
					max_num = num;
					filesFound = true;
				}
				else
				{
					min_num = std::min(min_num, num);
					max_num = std::max(max_num, num);
				}
			}
			catch (const std::exception&)
			{
#ifdef DEBUG
				Msg("[CSpecificCharacter::SetRandomRange]: Skip model with invalid numeric suffix: %s", name.c_str());
#endif
				continue;
			}
		}
	}

#ifdef DEBUG
	if (!filesFound)
		Msg("[CSpecificCharacter::SetRandomRange]: No valid numbered models found for visual name: %s, using default range [1,1]", visual_name.c_str());
#endif

	data()->first_visual = min_num;
	data()->last_visual = max_num;
}