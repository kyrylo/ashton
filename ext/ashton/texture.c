#include "texture.h"

extern VALUE rb_cPixelCache;

void Init_Ashton_Texture(VALUE module)
{
    rb_cTexture = rb_define_class_under(module, "Texture", rb_cObject);

    rb_define_alloc_func(rb_cTexture, texture_allocate);

    rb_define_method(rb_cTexture, "initialize_", Ashton_Texture_init, 2);

    rb_define_method(rb_cTexture, "cache", Ashton_Texture_get_cache, 0);

    rb_define_method(rb_cTexture, "width", Ashton_Texture_get_width, 0);
    rb_define_method(rb_cTexture, "height", Ashton_Texture_get_height, 0);
    rb_define_method(rb_cTexture, "fbo_id", Ashton_Texture_get_fbo_id, 0);
    rb_define_method(rb_cTexture, "id", Ashton_Texture_get_id, 0);

    rb_define_method(rb_cTexture, "[]", Ashton_Texture_get_pixel, 2);
    rb_define_method(rb_cTexture, "rgba", Ashton_Texture_get_rgba_array, 2);
    rb_define_method(rb_cTexture, "red", Ashton_Texture_get_red, 2);
    rb_define_method(rb_cTexture, "green", Ashton_Texture_get_green, 2);
    rb_define_method(rb_cTexture, "blue", Ashton_Texture_get_blue, 2);
    rb_define_method(rb_cTexture, "alpha", Ashton_Texture_get_alpha, 2);

    rb_define_method(rb_cTexture, "transparent?", Ashton_Texture_is_transparent, 2);
    rb_define_method(rb_cTexture, "refresh_cache", Ashton_Texture_refresh_cache, 0);
    rb_define_method(rb_cTexture, "to_blob", Ashton_Texture_to_blob, 0);
    rb_define_method(rb_cTexture, "draw", Ashton_Texture_draw, -1);
}

//
VALUE Ashton_Texture_get_width(VALUE self)
{
    TEXTURE();
    return UINT2NUM(texture->width);
}

//
VALUE Ashton_Texture_get_height(VALUE self)
{
    TEXTURE();
    return UINT2NUM(texture->height);
}

VALUE Ashton_Texture_get_fbo_id(VALUE self)
{
    TEXTURE();
    return UINT2NUM(texture->fbo_id);
}

VALUE Ashton_Texture_get_id(VALUE self)
{
    TEXTURE();
    return UINT2NUM(texture->id);
}

//
VALUE Ashton_Texture_init(VALUE self, VALUE width, VALUE height)
{
    TEXTURE();

    if(!GL_EXT_framebuffer_object)
    {
       rb_raise(rb_eRuntimeError, "Ashton::Texture requires GL_EXT_framebuffer_object, which is not supported by OpenGL");
    }

    texture->width = NUM2UINT(width);
    texture->height = NUM2UINT(height);

    texture->rb_cache = Qnil;

    // Create the FBO itself.
    glGenFramebuffersEXT(1, &texture->fbo_id);
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, texture->fbo_id);

    // Create the texture.
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // Create an empty texture, that might be filled with junk.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, texture->width,
                texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                              GL_TEXTURE_2D, texture->id, 0);

    // Make everything safe again
    glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    return Qnil;
}

//
static VALUE texture_allocate(VALUE klass)
{
    Texture* texture = ALLOC(Texture);
    memset(texture, 0, sizeof(Texture));

    return Data_Wrap_Struct(klass, texture_mark, texture_free, texture);
}

//
static void texture_mark(Texture* texture)
{
    if(!NIL_P(texture->rb_cache)) rb_gc_mark(texture->rb_cache);
}

//
static void texture_free(Texture* texture)
{
    glDeleteFramebuffersEXT(1, &texture->fbo_id);
    glDeleteTextures(1, &texture->id);
    xfree(texture);
}

//
static void ensure_cache_exists(VALUE self, Texture* texture)
{
    if(NIL_P(texture->rb_cache))
    {
        texture->rb_cache = rb_funcall(rb_cPixelCache, rb_intern("new"), 1, self);
    }
}

// Returns the cache. Creates it if necessary.
VALUE Ashton_Texture_get_cache(VALUE self)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return texture->rb_cache;
}

//
VALUE Ashton_Texture_get_pixel(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_pixel(texture->rb_cache, x, y);
}

VALUE Ashton_Texture_get_rgba_array(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_rgba_array(texture->rb_cache, x, y);
}

//
VALUE Ashton_Texture_get_red(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_red(texture->rb_cache, x, y);
}

//
VALUE Ashton_Texture_get_green(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_green(texture->rb_cache, x, y);
}

//
VALUE Ashton_Texture_get_blue(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_blue(texture->rb_cache, x, y);
}

//
VALUE Ashton_Texture_get_alpha(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_get_alpha(texture->rb_cache, x, y);
}

//
VALUE Ashton_Texture_is_transparent(VALUE self, VALUE x, VALUE y)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_is_transparent(texture->rb_cache, x, y);
}

VALUE Ashton_Texture_refresh_cache(VALUE self)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_refresh(texture->rb_cache);
}

VALUE Ashton_Texture_to_blob(VALUE self)
{
    TEXTURE();
    ensure_cache_exists(self, texture);
    return Ashton_PixelCache_to_blob(texture->rb_cache);
}

// ----------------------------------------
static VALUE draw_block(VALUE yield_value, VALUE parameters, int argc, VALUE argv[])
{
    VALUE self, x, y, blend_mode, color, shader;
    self = rb_ary_entry(parameters, 0);
    x = rb_ary_entry(parameters, 1);
    y = rb_ary_entry(parameters, 2);
    blend_mode = rb_ary_entry(parameters, 3);
    color = rb_ary_entry(parameters, 4);
    shader = rb_ary_entry(parameters, 5);

    TEXTURE();

    if(!NIL_P(shader))
    {
        VALUE options = rb_hash_new();
        rb_hash_aset(options, SYMBOL("required"), Qfalse);

        int location = rb_funcall(shader, rb_intern("send"), 3, SYMBOL("uniform_location"), SYMBOL("texture_enabled"), options);
        if(location != -1)
        {
            rb_funcall(shader, rb_intern("send"), 3, SYMBOL("set_uniform"), INT2NUM(location), Qtrue);
        }
        rb_funcall(shader, rb_intern("color="), 1, color);
    }

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, texture->id);

    // Set blending mode.
    ID blend_id = SYM2ID(blend_mode);

    if(blend_id == rb_intern("default") ||
       blend_id == rb_intern("alpha"))
    {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }
    else if(blend_id == rb_intern("additive") ||
            blend_id == rb_intern("add"))
    {
        glBlendFunc(GL_ONE, GL_ONE);
    }
    else if(blend_id == rb_intern("multiplicative") ||
            blend_id == rb_intern("multiply"))
    {
        glBlendFunc(GL_DST_COLOR, GL_ZERO);
    }
    else if(blend_id == rb_intern("copy"))
    {
        glBlendFunc(GL_ONE, GL_ZERO);
    }
    else
    {
        rb_raise(rb_eArgError, "Unrecognised blend mode: :%s",
                 rb_id2name(blend_id));
    }

    glBegin(GL_QUADS);
        glTexCoord2d(0, 0);
        glVertex2d(x, y + texture->height); // BL

        glTexCoord2d(0, 1);
        glVertex2d(x, y); // TL

        glTexCoord2d(1, 1);
        glVertex2d(x + texture->width, y); // TR

        glTexCoord2d(1, 0);
        glVertex2d(x + texture->width, y + texture->height); // BR
    glEnd();

    return Qnil;
}

// ----------------------------------------
VALUE Ashton_Texture_draw(int argc, VALUE argv[], VALUE self)
{
    TEXTURE();

    VALUE x, y, z, options;
    VALUE shader, blend_mode, color;

    if(rb_scan_args(argc, argv, "31", &x, &y, &z, &options) == 4)
    {
        // Get :shader
        shader = rb_hash_aref(options, SYMBOL("shader"));
        if(!NIL_P(shader))
        {
            if(!rb_obj_is_kind_of(shader, rb_cShader))
            {
               rb_raise(rb_eTypeError, "Expecting :shader to be an Ashton::Shader");
            }
        }

        // Get :blend mode
        blend_mode = rb_hash_aref(options, SYMBOL("blend"));
        if(NIL_P(blend_mode))
        {
            blend_mode = SYMBOL("alpha");
        }
        else
        {
            Check_Type(blend_mode, T_SYMBOL);
        }

        // Get :color
        color = rb_hash_aref(options, SYMBOL("color"));
        if(NIL_P(color))
        {
            color = UINT2NUM(0xffffffff);
        }
        else
        {
            if(!rb_obj_is_kind_of(color, rb_cColor))
            {
                rb_raise(rb_eTypeError, "Expecting :color to be a Gosu::Color");
            }
        }
    }
    else
    {
       shader = Qnil;
       blend_mode = SYMBOL("alpha");
       color = UINT2NUM(0xffffffff);
    }

    VALUE window = rb_gv_get("$window");

    VALUE block_argv[1];
    block_argv[0] = z;

    if(!NIL_P(shader))
    {
        rb_funcall(shader, rb_intern("enable"), 1, z);
    }

    VALUE parameters = rb_ary_new();
    rb_ary_push(parameters, self);
    rb_ary_push(parameters, x);
    rb_ary_push(parameters, y);
    rb_ary_push(parameters, blend_mode);
    rb_ary_push(parameters, color);
    if(!NIL_P(shader)) rb_ary_push(parameters, shader);

    rb_block_call(window, rb_intern("gl"), 1, block_argv,
                  RUBY_METHOD_FUNC(draw_block), parameters);

    // Disable the shader, if provided.
    if(!NIL_P(shader)) rb_funcall(shader, rb_intern("disable"), 1, z);
}