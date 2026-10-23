#include "gl_api.h"

bool SotnGlApi_Load(SotnGlApi *gl)
{
#define LOAD_GL(name)                                                                              \
    gl->name = (Gl##name)SDL_GL_GetProcAddress("gl" #name);                                        \
    if (gl->name == NULL)                                                                          \
    {                                                                                              \
        return SDL_SetError("Missing OpenGL function gl%s", #name);                                \
    }
    LOAD_GL(GetString);
    LOAD_GL(GetIntegerv);
    LOAD_GL(GetError);
    LOAD_GL(ClearColor);
    LOAD_GL(Clear);
    LOAD_GL(Viewport);
    LOAD_GL(Disable);
    LOAD_GL(Enable);
    LOAD_GL(BlendFunc);
    LOAD_GL(GenTextures);
    LOAD_GL(DeleteTextures);
    LOAD_GL(BindTexture);
    LOAD_GL(TexImage2D);
    LOAD_GL(TexParameteri);
    LOAD_GL(PixelStorei);
    LOAD_GL(ActiveTexture);
    LOAD_GL(CreateShader);
    LOAD_GL(ShaderSource);
    LOAD_GL(CompileShader);
    LOAD_GL(GetShaderiv);
    LOAD_GL(GetShaderInfoLog);
    LOAD_GL(DeleteShader);
    LOAD_GL(CreateProgram);
    LOAD_GL(AttachShader);
    LOAD_GL(BindAttribLocation);
    LOAD_GL(LinkProgram);
    LOAD_GL(GetProgramiv);
    LOAD_GL(GetProgramInfoLog);
    LOAD_GL(DeleteProgram);
    LOAD_GL(UseProgram);
    LOAD_GL(GetUniformLocation);
    LOAD_GL(Uniform1i);
    LOAD_GL(GenVertexArrays);
    LOAD_GL(BindVertexArray);
    LOAD_GL(DeleteVertexArrays);
    LOAD_GL(GenBuffers);
    LOAD_GL(BindBuffer);
    LOAD_GL(BufferData);
    LOAD_GL(DeleteBuffers);
    LOAD_GL(EnableVertexAttribArray);
    LOAD_GL(VertexAttribPointer);
    LOAD_GL(DrawArrays);
    LOAD_GL(ReadPixels);
#undef LOAD_GL
    return true;
}
