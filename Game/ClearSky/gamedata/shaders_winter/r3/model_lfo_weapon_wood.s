local tex_rust = "shaders\\layers\\model_snow"
local tex_rust_bump = "shaders\\layers\\model_snow_bump"

function normal(shader, t_base, t_second, t_detail)
	shader:begin("deffer_model_hud_bump","deffer_lfo_weapons_rust")
	: fog (false)	
	shader:dx10stencil(true, cmp_func.always, 255 , 127, stencil_op.keep, stencil_op.replace, stencil_op.keep)
	shader:dx10stencil_ref(1)		
	
	shader:dx10texture("s_base", t_base)
	shader:dx10texture("s_bump", t_base.."_bump")	
	shader:dx10texture("s_bumpX", t_base.."_bump#")	
	shader:dx10texture("s_base_rust", tex_rust)		
	shader:dx10texture("s_base_rust_bump", tex_rust_bump)	
	shader:dx10sampler("smp_base")	
end
