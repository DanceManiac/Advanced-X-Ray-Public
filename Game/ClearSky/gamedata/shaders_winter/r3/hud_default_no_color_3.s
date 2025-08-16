function normal(shader, t_base, t_second, t_detail)
	shader:begin("stub_notransform_t","hud_default_no_color_3")
		: fog(false)
		: zb(false,false)
		: blend(true,blend.srcalpha,blend.invsrcalpha)	
	shader:dx10texture("s_base", t_base)
	shader:dx10sampler("smp_base")
end