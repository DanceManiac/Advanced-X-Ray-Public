function normal(shader, t_base, t_second, t_detail)
	shader	: begin	("effects_new_aref","effects_new_aref")
			: zb	(true,false)
			: blend	(true,blend.srcalpha,blend.invsrcalpha)
			: aref 		(true,0)

	shader : dx10texture("s_base", "fx\\fx_snow")
	shader : dx10texture("s_position", "$user$position")
	shader : dx10texture("s_accumulator", "$user$accum")
	shader : dx10texture("s_blur_4", "$user$blur_4")

	shader 	: dx10sampler	("smp_rtlinear")
	shader 	: dx10sampler	("smp_nofilter")
end