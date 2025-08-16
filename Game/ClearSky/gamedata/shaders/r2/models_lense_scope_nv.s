
function normal   (shader, t_base, t_second, t_detail)
	shader:begin	("model_scope_lense","model_scope_lense_nv")
      : fog			(true)
      : zb			(true,false)
      : blend		(true,blend.srcalpha,blend.invsrcalpha)
      : aref		(true,0)
      : sorting		(2,true)
      : distort		(true)
		shader:sampler ("s_base")		:texture	(t_base) :clamp()

		shader:sampler ("s_vp2")		:texture	("$user$viewport2")
		shader:sampler ("s_position")	:texture	("$user$position")
end
