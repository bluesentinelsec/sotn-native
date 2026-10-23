#ifndef SOTN_PRESENTATION_SHADERS_H
#define SOTN_PRESENTATION_SHADERS_H

#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
#define SOTN_SHADER_VERSION "#version 300 es\nprecision highp float;\n"
#else
#define SOTN_SHADER_VERSION "#version 150 core\n"
#endif

static const char vertex_source[] = SOTN_SHADER_VERSION
    "in vec2 position;\n"
    "in vec2 uv;\n"
    "out vec2 texture_uv;\n"
    "void main() { gl_Position = vec4(position, 0.0, 1.0); texture_uv = uv; }\n";

static const char fragment_source[] =
    SOTN_SHADER_VERSION "uniform sampler2D image;\n"
                        "in vec2 texture_uv;\n"
                        "out vec4 output_color;\n"
                        "void main() { output_color = texture(image, texture_uv); }\n";

#undef SOTN_SHADER_VERSION
#endif
