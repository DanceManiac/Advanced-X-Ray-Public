////////////////////////////////////////////////////////////////////////////
//	Module 		: editor_environment_effects_effect.cpp
//	Created 	: 28.12.2007
//  Modified 	: 28.12.2007
//	Author		: Dmitriy Iassenev
//	Description : editor environment effects effect class
////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"

#ifdef INGAME_EDITOR

#include "editor_environment_effects_effect.hpp"
#include "../include/editor/property_holder.hpp"
#include "editor_environment_manager.hpp"
#include "ide.hpp"
#include "editor_environment_effects_manager.hpp"
#include "editor_environment_detail.hpp"

using editor::environment::effects::effect;
using editor::environment::effects::manager;

effect::effect			(manager const& manager, shared_str const& id) :
	m_manager			(manager),
	m_property_holder	(0),
	m_id				(id),
	m_sound_eff			("")
{
	m_particles			= "";
}

effect::~effect			()
{
	if (!Device.editor())
		return;

	::ide().destroy		(m_property_holder);
}

void effect::load		(CInifile& config)
{
	m_life_time			= config.r_u32			(m_id, "life_time");
	m_offset			= config.r_fvector3		(m_id, "offset");
	m_particles			= config.r_string		(m_id, "particles");
	m_sound_eff			= config.r_string		(m_id, "sound");
	m_wind_gust_factor	= config.r_float		(m_id, "wind_gust_factor");
}

void effect::save		(CInifile& config)
{
	config.w_u32		(m_id.c_str(), "life_time",				m_life_time				);
	config.w_fvector3	(m_id.c_str(), "offset",				m_offset				);
	config.w_string		(m_id.c_str(), "particles",				m_particles.c_str()		);
	config.w_string		(m_id.c_str(), "sound",					m_sound_eff.c_str()			);
	config.w_float		(m_id.c_str(), "wind_gust_factor",		m_wind_gust_factor		);
	config.w_float		(m_id.c_str(), "wind_blast_in_time",	m_wind_blast_in_time	);
	config.w_float		(m_id.c_str(), "wind_blast_out_time",	m_wind_blast_out_time	);
	config.w_float		(m_id.c_str(), "wind_blast_strength",	m_wind_blast_strength	);
	config.w_float		(m_id.c_str(), "wind_blast_longitude",	rad2deg(m_wind_blast_direction.getH()));
}

LPCSTR effect::id_getter() const
{
	return				(m_id.c_str());
}

void effect::id_setter	(LPCSTR value_)
{
	shared_str			value = value_;
	if (m_id._get() == value._get())
		return;

	m_id				= m_manager.unique_id(value);
}

LPCSTR effect::sound_getter		()
{
	return				(m_sound_eff.c_str());
}

void effect::sound_setter		(LPCSTR value)
{
	m_sound_eff			= value;
	m_sound.destroy		();
	m_sound.create		(value, st_Effect, sg_SourceType);
}

float effect::wind_blast_longitude_getter	() const
{
	float						h, p;
	m_wind_blast_direction.getHP	(h, p);
	return						(rad2deg(h));
}

void effect::wind_blast_longitude_setter	(float value)
{
	m_wind_blast_direction.setHP	(deg2rad(value), 0.f);
}

void effect::fill		(editor::property_holder_collection* collection)
{
	VERIFY				(!m_property_holder);
	m_property_holder	= ::ide().create_property_holder(m_id.c_str(), collection, this);

	typedef editor::property_holder::string_getter_type	string_getter_type;
	string_getter_type	string_getter;
	string_getter.bind	(this, &effect::id_getter);

	typedef editor::property_holder::string_setter_type	string_setter_type;
	string_setter_type	string_setter;
	string_setter.bind	(this, &effect::id_setter);

	m_property_holder->add_property	(
		"id",
		"properties",
		"this option is resposible for effect identifier",
		m_id.c_str(),
		string_getter,
		string_setter
	);
	m_property_holder->add_property	(
		"life time",
		"properties",
		"this option is resposible for effect life time (in milliseconds)",
		(int const&)m_life_time,
		(int&)m_life_time
	);
	m_property_holder->add_property	(
		"offset",
		"properties",
		"this option is resposible for effect offset (3D vector)",
		(vec3f const&)m_offset,
		(vec3f&)m_offset
	);
	m_property_holder->add_property(
		"particles",
		"properties",
		"this option is resposible for effect particles",
		m_particles.c_str(),
		m_particles,
		&*m_manager.environment().particle_ids().begin(),
		m_manager.environment().particle_ids().size(),
		editor::property_holder::value_editor_tree_view,
		editor::property_holder::cannot_enter_text
	);

	string_getter.bind	(this, &effect::sound_getter);
	string_setter.bind	(this, &effect::sound_setter);
	m_property_holder->add_property(
		"sound",
		"properties",
		"this option is resposible for effect sound",
		m_sound_eff.c_str(),
		string_getter,
		string_setter,
		".ogg",
		"Sound files (*.ogg)|*.ogg",
		detail::real_path("$game_sounds$", "").c_str(),
		"Select sound...",
		editor::property_holder::cannot_enter_text,
		editor::property_holder::remove_extension
	);
	m_property_holder->add_property	(
		"wind gust factor",
		"properties",
		"this option is resposible for effect wind gust factor",
		m_wind_gust_factor,
		m_wind_gust_factor
	);
	m_property_holder->add_property	(
		"wind blast strength",
		"properties",
		"this option is resposible for effect wind blast strength",
		m_wind_blast_strength,
		m_wind_blast_strength
	);
	m_property_holder->add_property	(
		"wind blast start time",
		"properties",
		"this option is resposible for effect wind blast start time",
		m_wind_blast_in_time,
		m_wind_blast_in_time,
		0.f,
		1000.f
	);
	m_property_holder->add_property	(
		"wind blast stop time",
		"properties",
		"this option is resposible for effect wind blast stop time",
		m_wind_blast_out_time,
		m_wind_blast_out_time,
		0.f,
		1000.f
	);

	typedef ::editor::property_holder::float_getter_type	float_getter_type;
	float_getter_type	float_getter;

	typedef ::editor::property_holder::float_setter_type	float_setter_type;
	float_setter_type	float_setter;

	float_getter.bind	(this, &effect::wind_blast_longitude_getter);
	float_setter.bind	(this, &effect::wind_blast_longitude_setter);
	m_property_holder->add_property	(
		"wind blast longitude",
		"properties",
		"this option is resposible for effect wind blast longitude",
		float_getter(),
		float_getter,
		float_setter,
		-360.f,
		 360.f
	);
}

editor::property_holder* effect::object	()
{
	return				(m_property_holder);
}

#endif // #ifdef INGAME_EDITOR