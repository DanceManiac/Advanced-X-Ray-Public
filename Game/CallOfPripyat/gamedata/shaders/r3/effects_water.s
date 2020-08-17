local tex_base                	= "water\\water_water"
local tex_randir                = "shaders\\randir"
local tex_base_s                = "water\\water_water_r1"
local tex_nmap                	= "water\\pure_waters_4"
local tex_nmap2                	= "water\\water_normal_12345"
local tex_dist                	= "water\\water_dudv"
local tex_leaves             	= "water\\water_listya"
local tex_foam              	= "water\\water_foam"
local tex_dudv                	= "water\\pure_waters_4_dudv"
local tex_nmap_empty			= "water\\water_normal_empty"
local tex_dmap_empty			= "water\\water_dudv_empty"
local tex_cubemap_prip			= "water\\water_prip"
local tex_nmap_noise            = "water\\water_normal_12345"
local tex_env0                	= "$user$sky0"      
local tex_env1                	= "$user$sky1"        

function normal                (shader, t_base, t_second, t_detail)
	shader	:begin		("water_improved","water_improved")
    		:sorting	(2, false)
			:blend		(true,blend.srcalpha,blend.invsrcalpha)
			:zb			(true,true)
			:distort	(true)
			:fog		(true)

	shader:dx10texture	("s_randir",		tex_randir)
	shader:dx10texture	("s_base",		tex_base)
	shader:dx10texture	("s_base_static",	tex_base_s)
	shader:dx10texture	("s_nmap",		tex_nmap)
	shader:dx10texture	("s_nmap_blurred", tex_nmap2)
	shader:dx10texture	("s_nmap_empty", tex_nmap_empty)
	shader:dx10texture	("s_dmap_empty", tex_dmap_empty)
	shader:dx10texture	("s_env0",		tex_env0)
	shader:dx10texture	("s_env1",		tex_env1)
	shader:dx10texture	("sky_s0",		tex_env0)
	shader:dx10texture	("sky_s1",		tex_env1)
	shader:dx10texture	("s_position",	"$user$position")
	shader:dx10texture	("s_dudv_water_map", tex_dudv)

	shader:dx10texture	("s_leaves",	tex_leaves)
	shader:dx10texture	("s_foam",		tex_foam)
	
	shader:dx10texture	("s_pptemp", "$user$genericpp")
	
	shader:dx10sampler	("smp_base")
	shader:dx10sampler	("smp_nofilter")
	shader:dx10sampler	("smp_rtlinear")
	shader:dx10sampler	("smp_linear")
end

function l_special        (shader, t_base, t_second, t_detail)
	shader	:begin                ("water_improved","waterd_distort")
			:sorting        (2, true)
			:blend                (true,blend.srcalpha,blend.invsrcalpha)
			:zb                (true,false)
			:fog                (false)
			:distort        (true)

	shader: dx10color_write_enable( true, true, true, false)

	shader:dx10texture	("s_nmap",		tex_nmap)
	
	shader:dx10sampler	("smp_base")
end