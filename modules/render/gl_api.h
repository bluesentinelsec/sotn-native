#ifndef SOTN_GL_API_H
#define SOTN_GL_API_H

#include <SDL3/SDL.h>
#if defined(SDL_PLATFORM_IOS)
#include <OpenGLES/ES3/gl.h>
#elif defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_EMSCRIPTEN)
#include <GLES3/gl3.h>
#else
#include <SDL3/SDL_opengl.h>
#endif

#ifndef APIENTRY
#define APIENTRY GL_APIENTRY
#endif

typedef const GLubyte *(APIENTRY *GlGetString)(GLenum name);
typedef void(APIENTRY *GlGetIntegerv)(GLenum name, GLint *value);
typedef GLenum(APIENTRY *GlGetError)(void);
typedef void(APIENTRY *GlClearColor)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
typedef void(APIENTRY *GlClear)(GLbitfield mask);
typedef void(APIENTRY *GlViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
typedef void(APIENTRY *GlDisable)(GLenum capability);
typedef void(APIENTRY *GlEnable)(GLenum capability);
typedef void(APIENTRY *GlBlendFunc)(GLenum source, GLenum destination);
typedef void(APIENTRY *GlGenTextures)(GLsizei count, GLuint *textures);
typedef void(APIENTRY *GlDeleteTextures)(GLsizei count, const GLuint *textures);
typedef void(APIENTRY *GlBindTexture)(GLenum target, GLuint texture);
typedef void(APIENTRY *GlTexImage2D)(GLenum target, GLint level, GLint internal_format,
                                     GLsizei width, GLsizei height, GLint border, GLenum format,
                                     GLenum type, const void *pixels);
typedef void(APIENTRY *GlTexParameteri)(GLenum target, GLenum name, GLint value);
typedef void(APIENTRY *GlPixelStorei)(GLenum name, GLint value);
typedef void(APIENTRY *GlActiveTexture)(GLenum texture);
typedef GLuint(APIENTRY *GlCreateShader)(GLenum type);
typedef void(APIENTRY *GlShaderSource)(GLuint shader, GLsizei count, const GLchar *const *strings,
                                       const GLint *lengths);
typedef void(APIENTRY *GlCompileShader)(GLuint shader);
typedef void(APIENTRY *GlGetShaderiv)(GLuint shader, GLenum name, GLint *value);
typedef void(APIENTRY *GlGetShaderInfoLog)(GLuint shader, GLsizei capacity, GLsizei *length,
                                           GLchar *output);
typedef void(APIENTRY *GlDeleteShader)(GLuint shader);
typedef GLuint(APIENTRY *GlCreateProgram)(void);
typedef void(APIENTRY *GlAttachShader)(GLuint program, GLuint shader);
typedef void(APIENTRY *GlBindAttribLocation)(GLuint program, GLuint index, const GLchar *name);
typedef void(APIENTRY *GlLinkProgram)(GLuint program);
typedef void(APIENTRY *GlGetProgramiv)(GLuint program, GLenum name, GLint *value);
typedef void(APIENTRY *GlGetProgramInfoLog)(GLuint program, GLsizei capacity, GLsizei *length,
                                            GLchar *output);
typedef void(APIENTRY *GlDeleteProgram)(GLuint program);
typedef void(APIENTRY *GlUseProgram)(GLuint program);
typedef GLint(APIENTRY *GlGetUniformLocation)(GLuint program, const GLchar *name);
typedef void(APIENTRY *GlUniform1i)(GLint location, GLint value);
typedef void(APIENTRY *GlGenVertexArrays)(GLsizei count, GLuint *arrays);
typedef void(APIENTRY *GlBindVertexArray)(GLuint array);
typedef void(APIENTRY *GlDeleteVertexArrays)(GLsizei count, const GLuint *arrays);
typedef void(APIENTRY *GlGenBuffers)(GLsizei count, GLuint *buffers);
typedef void(APIENTRY *GlBindBuffer)(GLenum target, GLuint buffer);
typedef void(APIENTRY *GlBufferData)(GLenum target, GLsizeiptr size, const void *data,
                                     GLenum usage);
typedef void(APIENTRY *GlDeleteBuffers)(GLsizei count, const GLuint *buffers);
typedef void(APIENTRY *GlEnableVertexAttribArray)(GLuint index);
typedef void(APIENTRY *GlVertexAttribPointer)(GLuint index, GLint size, GLenum type,
                                              GLboolean normalized, GLsizei stride,
                                              const void *pointer);
typedef void(APIENTRY *GlDrawArrays)(GLenum mode, GLint first, GLsizei count);
typedef void(APIENTRY *GlReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
                                     GLenum type, void *pixels);

typedef struct SotnGlApi
{
    GlGetString GetString;
    GlGetIntegerv GetIntegerv;
    GlGetError GetError;
    GlClearColor ClearColor;
    GlClear Clear;
    GlViewport Viewport;
    GlDisable Disable;
    GlEnable Enable;
    GlBlendFunc BlendFunc;
    GlGenTextures GenTextures;
    GlDeleteTextures DeleteTextures;
    GlBindTexture BindTexture;
    GlTexImage2D TexImage2D;
    GlTexParameteri TexParameteri;
    GlPixelStorei PixelStorei;
    GlActiveTexture ActiveTexture;
    GlCreateShader CreateShader;
    GlShaderSource ShaderSource;
    GlCompileShader CompileShader;
    GlGetShaderiv GetShaderiv;
    GlGetShaderInfoLog GetShaderInfoLog;
    GlDeleteShader DeleteShader;
    GlCreateProgram CreateProgram;
    GlAttachShader AttachShader;
    GlBindAttribLocation BindAttribLocation;
    GlLinkProgram LinkProgram;
    GlGetProgramiv GetProgramiv;
    GlGetProgramInfoLog GetProgramInfoLog;
    GlDeleteProgram DeleteProgram;
    GlUseProgram UseProgram;
    GlGetUniformLocation GetUniformLocation;
    GlUniform1i Uniform1i;
    GlGenVertexArrays GenVertexArrays;
    GlBindVertexArray BindVertexArray;
    GlDeleteVertexArrays DeleteVertexArrays;
    GlGenBuffers GenBuffers;
    GlBindBuffer BindBuffer;
    GlBufferData BufferData;
    GlDeleteBuffers DeleteBuffers;
    GlEnableVertexAttribArray EnableVertexAttribArray;
    GlVertexAttribPointer VertexAttribPointer;
    GlDrawArrays DrawArrays;
    GlReadPixels ReadPixels;
} SotnGlApi;

bool SotnGlApi_Load(SotnGlApi *gl);

#endif
