function normal		(shader, t_base, t_second, t_detail)
	shader:begin	("deffer_model_flat","deffer_base_flat")
			: fog		(false)
			: emissive 	(true)
	shader:sampler	("s_base")      :texture	(t_base)
end

function l_special	(shader, t_base, t_second, t_detail)
	shader:begin	("deffer_model_flat",	"accum_advanced_emissive")
			: zb 		(true,false)
			: fog		(false)
			: emissive 	(true)
			
	shader:sampler	("s_base")      :texture	(t_base)
	shader:sampler	("s_lightmap", t_base.. "_lightmap")
end
