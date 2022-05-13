function normal(shader, t_base, t_second, t_detail)
	shader : begin("forward_rain","forward_rain")
	: zb(true,false)
	: blend(true,blend.srcalpha,blend.invsrcalpha)
	: aref(true,0)
	: sorting(2, true)	
	
	shader : dx10texture("s_base", t_base)
	shader : dx10texture("s_bump", t_base.."_bump")	
	shader : dx10texture("s_image", "$user$genericpp")
	shader : dx10texture("env_s0", "$user$env_s0")
	shader : dx10texture("env_s1", "$user$env_s1")
	  
	shader : dx10sampler("smp_base")
	shader : dx10sampler("smp_rtlinear")
end