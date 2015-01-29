/*********************************************************************************************
 * Register list of OpenGL programs
 * Register Macro: CCPLUS_PROGRAM ( programName, vertexShader, fragmentShader, setupFunction)
 *********************************************************************************************/

#define SETUP_FUNC_DECL [](GLuint program, std::vector<GLuint>& uniforms)
#define SET_UNIFORM(NAME, VALUE) glUniform1i(glGetUniformLocation(program, #NAME), VALUE)
#define CACHE_UNIFORM(NAME) uniforms.push_back(glGetUniformLocation(program, #NAME))


// Blend mode programs
#define BLEND_PROGRAM(X) CCPLUS_PROGRAM(blend_ ## X, "shaders/fill.v.glsl", "shaders/blenders/" #X ".f.glsl", \
        SETUP_FUNC_DECL { SET_UNIFORM(tex_up, 1); SET_UNIFORM(tex_down, 2); })
BLEND_PROGRAM( none )
BLEND_PROGRAM( default )
BLEND_PROGRAM( add )
BLEND_PROGRAM( multiply )
BLEND_PROGRAM( screen )
BLEND_PROGRAM( disolve )
BLEND_PROGRAM( darken )
BLEND_PROGRAM( lighten )
BLEND_PROGRAM( overlay )
BLEND_PROGRAM( difference )
#undef BLEND_PROGRAM

// Track matte programs
#define TRKMTE_PROGRAM(X) CCPLUS_PROGRAM(trkmte_ ## X, "shaders/fill.v.glsl", "shaders/trkmte/" #X ".f.glsl", \
        SETUP_FUNC_DECL { SET_UNIFORM(tex_up, 1); SET_UNIFORM(tex_down, 2); })
TRKMTE_PROGRAM( alpha )
TRKMTE_PROGRAM( alpha_inv )
TRKMTE_PROGRAM( luma )
TRKMTE_PROGRAM( luma_inv )
#undef TRKMTE_PROGRAM

// Filter programs
CCPLUS_PROGRAM( filter_transform, "shaders/filters/transform.v.glsl", "shaders/filters/transform.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(trans);
    CACHE_UNIFORM(src_dst_size);
    CACHE_UNIFORM(zoom);
    CACHE_UNIFORM(alpha);
})

CCPLUS_PROGRAM( filter_4corner, "shaders/filters/4corner.v.glsl", "shaders/plain.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(trans);
    CACHE_UNIFORM(src_dst_size);
    CACHE_UNIFORM(transition);
})

CCPLUS_PROGRAM( filter_4color, "shaders/fill.v.glsl", "shaders/filters/4color.f.glsl", SETUP_FUNC_DECL {
    CACHE_UNIFORM(opacity);
    CACHE_UNIFORM(params_b);
    CACHE_UNIFORM(params_g);
    CACHE_UNIFORM(params_r);
})

CCPLUS_PROGRAM( filter_gaussian, "shaders/filters/gaussian.v.glsl", "shaders/filters/gaussian.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(ksize);
    CACHE_UNIFORM(gWeights);
    CACHE_UNIFORM(gOffsets);
    CACHE_UNIFORM(pixelOffset);
})

CCPLUS_PROGRAM( filter_hsl, "shaders/fill.v.glsl", "shaders/filters/hsl.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(hue);
    CACHE_UNIFORM(sat);
    CACHE_UNIFORM(lit);
})

CCPLUS_PROGRAM( filter_mask, "shaders/fill.v.glsl", "shaders/filters/mask.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
})

CCPLUS_PROGRAM( filter_mask_gen, "shaders/fill.v.glsl", "shaders/filters/mask_gen.f.glsl", SETUP_FUNC_DECL {
})

CCPLUS_PROGRAM( filter_mask_merge, "shaders/fill.v.glsl", "shaders/filters/mask_merge.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex_up, 1);
    SET_UNIFORM(tex_down, 2);
})

CCPLUS_PROGRAM( filter_opacity, "shaders/fill.v.glsl", "shaders/filters/opacity.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(opa);
})

CCPLUS_PROGRAM( filter_grayscale, "shaders/fill.v.glsl", "shaders/filters/grayscale.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
    CACHE_UNIFORM(weights);
    CACHE_UNIFORM(hue_sat);
})

CCPLUS_PROGRAM( filter_ramp_linear, "shaders/fill.v.glsl", "shaders/filters/ramp_linear.f.glsl", SETUP_FUNC_DECL {
    CACHE_UNIFORM(alpha);
    CACHE_UNIFORM(dis);
    CACHE_UNIFORM(start);
    CACHE_UNIFORM(end);
    CACHE_UNIFORM(s_rgb);
    CACHE_UNIFORM(e_rgb);
})

CCPLUS_PROGRAM( filter_ramp_radial, "shaders/fill.v.glsl", "shaders/filters/ramp_radial.f.glsl", SETUP_FUNC_DECL {
    CACHE_UNIFORM(alpha);
    CACHE_UNIFORM(dis);
    CACHE_UNIFORM(start);
    CACHE_UNIFORM(end);
    CACHE_UNIFORM(s_rgb);
    CACHE_UNIFORM(e_rgb);
})

CCPLUS_PROGRAM( alpha_premultiply, "shaders/fill.v.glsl", "shaders/alpha_premultiply.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex, 0);
})

CCPLUS_PROGRAM( motion_blur_acuum, "shaders/fill.v.glsl", "shaders/mtblur.f.glsl", SETUP_FUNC_DECL {
    SET_UNIFORM(tex_up, 1);
    SET_UNIFORM(tex_down, 2);
    CACHE_UNIFORM(ratio);
})

#undef SETUP_FUNC_DECL
#undef SET_UNIFORM
#undef CACHE_UNIFORM
