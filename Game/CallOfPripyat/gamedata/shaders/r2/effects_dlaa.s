function normal(shader, t_base, t_second, t_detail)
    shader:begin("null", "dlaa_main")
        :fog        (false)
        :zb        (false, false)
    shader:sampler    ("s_image") :texture("$user$generic0") :clamp() :f_linear()
end