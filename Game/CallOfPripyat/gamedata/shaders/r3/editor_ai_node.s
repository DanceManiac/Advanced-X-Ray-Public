function normal(shader, t_base, t_second, t_detail)
	shader:begin("editor_ai_node", "editor_ai_node")
		:fog(false)
		:zb(true, false)
		:blend(true, blend.srcalpha, blend.invsrcalpha)
		:sorting(2, false)
		
	shader:dx10texture("s_base", t_base)
    shader:dx10sampler("smp_base")
end