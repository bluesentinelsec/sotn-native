#include "gl_api.h"
#include "layout.h"
#include "shaders.h"
#include <sotn/presenter.h>

struct SotnPresenter
{
    SDL_Window *window;
    SDL_GLContext context;
    SotnGlApi gl;
    GLuint program;
    GLuint texture;
    GLuint vertices;
    GLuint vertex_array;
    int width;
    int height;
};

static bool CheckGl(SotnPresenter *presenter, const char *operation)
{
    const GLenum error = presenter->gl.GetError();
    return error == GL_NO_ERROR ||
           SDL_SetError("%s: OpenGL error 0x%x", operation, (unsigned int)error);
}

static bool DestroyAfterError(SotnPresenter *presenter)
{
    char detail[1024] = {0};
    SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
    SotnPresenter_Destroy(presenter);
    return SDL_SetError("%s", detail);
}

static GLuint CompileShader(SotnPresenter *presenter, GLenum kind, const char *source)
{
    SotnGlApi *gl = &presenter->gl;
    const GLuint shader = gl->CreateShader(kind);
    if (shader == 0)
    {
        SDL_SetError("Cannot create presentation shader");
        return 0;
    }
    gl->ShaderSource(shader, 1, &source, NULL);
    gl->CompileShader(shader);
    GLint compiled = 0;
    gl->GetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled)
    {
        char detail[1024] = {0};
        gl->GetShaderInfoLog(shader, sizeof(detail), NULL, detail);
        gl->DeleteShader(shader);
        SDL_SetError("Presentation shader compilation failed: %s", detail);
        return 0;
    }
    return shader;
}

static bool CreateResources(SotnPresenter *presenter)
{
    SotnGlApi *gl = &presenter->gl;
    const GLuint vertex = CompileShader(presenter, GL_VERTEX_SHADER, vertex_source);
    if (vertex == 0)
    {
        return false;
    }
    const GLuint fragment = CompileShader(presenter, GL_FRAGMENT_SHADER, fragment_source);
    if (fragment == 0)
    {
        gl->DeleteShader(vertex);
        return false;
    }
    presenter->program = gl->CreateProgram();
    if (presenter->program != 0)
    {
        gl->AttachShader(presenter->program, vertex);
        gl->AttachShader(presenter->program, fragment);
        gl->BindAttribLocation(presenter->program, 0, "position");
        gl->BindAttribLocation(presenter->program, 1, "uv");
        gl->LinkProgram(presenter->program);
    }
    gl->DeleteShader(vertex);
    gl->DeleteShader(fragment);
    if (presenter->program == 0)
    {
        return SDL_SetError("Cannot create presentation shader program");
    }
    GLint linked = 0;
    gl->GetProgramiv(presenter->program, GL_LINK_STATUS, &linked);
    if (!linked)
    {
        char detail[1024] = {0};
        gl->GetProgramInfoLog(presenter->program, sizeof(detail), NULL, detail);
        return SDL_SetError("Presentation shader link failed: %s", detail);
    }
    gl->UseProgram(presenter->program);
    const GLint image = gl->GetUniformLocation(presenter->program, "image");
    if (image < 0)
    {
        return SDL_SetError("Presentation shader has no image sampler");
    }
    gl->Uniform1i(image, 0);
    /* Source rows start at the top. Assign UV zero to the top vertices explicitly. */
    static const GLfloat vertices[] = {-1, 1, 0, 0, -1, -1, 0, 1, 1, 1,  1, 0,
                                       1,  1, 1, 0, -1, -1, 0, 1, 1, -1, 1, 1};
    gl->GenVertexArrays(1, &presenter->vertex_array);
    gl->BindVertexArray(presenter->vertex_array);
    gl->GenBuffers(1, &presenter->vertices);
    gl->BindBuffer(GL_ARRAY_BUFFER, presenter->vertices);
    gl->BufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    gl->EnableVertexAttribArray(0);
    gl->VertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), NULL);
    gl->EnableVertexAttribArray(1);
    gl->VertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat),
                            (const void *)(uintptr_t)(2 * sizeof(GLfloat)));
    gl->Enable(GL_BLEND);
    gl->BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    return CheckGl(presenter, "Cannot create presentation resources");
}

bool SotnPresenter_Create(const char *title, int width, int height, SotnPresenter **out)
{
    if (out == NULL)
    {
        return SDL_InvalidParamError("out");
    }
    *out = NULL;
    if (title == NULL || width <= 0 || height <= 0)
    {
        return SDL_SetError("Invalid presentation title or window dimensions");
    }
    SDL_GL_ResetAttributes();
#if defined(SDL_PLATFORM_ANDROID) || defined(SDL_PLATFORM_IOS) || defined(SDL_PLATFORM_EMSCRIPTEN)
    const int profile = SDL_GL_CONTEXT_PROFILE_ES;
    const int minor = 0;
#else
    const int profile = SDL_GL_CONTEXT_PROFILE_CORE;
    const int minor = 2;
#endif
    if (!SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, profile) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3) ||
        !SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, minor) ||
        !SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8) || !SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8) ||
        !SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8) || !SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1) ||
        !SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8))
    {
        return false;
    }
    SotnPresenter *presenter = SDL_calloc(1, sizeof(*presenter));
    if (presenter == NULL)
    {
        return false;
    }
    presenter->window =
        SDL_CreateWindow(title, width, height,
                         SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (presenter->window != NULL)
    {
        presenter->context = SDL_GL_CreateContext(presenter->window);
    }
    if (presenter->context == NULL || !SotnGlApi_Load(&presenter->gl))
    {
        return DestroyAfterError(presenter);
    }
    GLint major = 0, actual_minor = 0;
    presenter->gl.GetIntegerv(GL_MAJOR_VERSION, &major);
    presenter->gl.GetIntegerv(GL_MINOR_VERSION, &actual_minor);
    if (major < 3 || (major == 3 && actual_minor < minor))
    {
        SotnPresenter_Destroy(presenter);
        return SDL_SetError("OpenGL context is below required version 3.%d", minor);
    }
    SDL_Log("Presentation: %s / %s", (const char *)presenter->gl.GetString(GL_VERSION),
            (const char *)presenter->gl.GetString(GL_RENDERER));
    if (!CreateResources(presenter))
    {
        return DestroyAfterError(presenter);
    }
    /* Driver swap pacing is optional; SDL's callback rate owns application frame pacing. */
    SDL_GL_SetSwapInterval(0);
    *out = presenter;
    return true;
}

bool SotnPresenter_SetImage(SotnPresenter *presenter, const Uint8 *pixels, size_t size, int width,
                            int height)
{
    if (presenter == NULL || pixels == NULL || width <= 0 || height <= 0 ||
        (size_t)height > (64U * 1024U * 1024U) / 4U / (size_t)width ||
        size < (size_t)width * (size_t)height * 4U)
    {
        return SDL_SetError("Invalid presentation image dimensions, capacity, or 64 MiB budget");
    }
    if (!SDL_GL_MakeCurrent(presenter->window, presenter->context))
    {
        return false;
    }
    SotnGlApi *gl = &presenter->gl;
    GLint maximum = 0;
    gl->GetIntegerv(GL_MAX_TEXTURE_SIZE, &maximum);
    if (width > maximum || height > maximum)
    {
        return SDL_SetError("Presentation image exceeds the driver's texture limit");
    }
    GLuint texture = 0;
    gl->GenTextures(1, &texture);
    gl->ActiveTexture(GL_TEXTURE0);
    gl->BindTexture(GL_TEXTURE_2D, texture);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    gl->TexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    gl->PixelStorei(GL_UNPACK_ALIGNMENT, 1);
    gl->TexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (!CheckGl(presenter, "Cannot upload presentation image"))
    {
        gl->DeleteTextures(1, &texture);
        return false;
    }
    gl->DeleteTextures(1, &presenter->texture);
    presenter->texture = texture;
    presenter->width = width;
    presenter->height = height;
    return true;
}

static bool DrawFrame(SotnPresenter *presenter, bool swap)
{
    if (presenter == NULL || presenter->width <= 0 || presenter->height <= 0)
    {
        return SDL_SetError("No presentation image has been uploaded");
    }
    if (SDL_GetWindowFlags(presenter->window) & SDL_WINDOW_MINIMIZED)
        return true;
    if (!SDL_GL_MakeCurrent(presenter->window, presenter->context))
    {
        return false;
    }
    int width = 0, height = 0;
    if (!SDL_GetWindowSizeInPixels(presenter->window, &width, &height))
    {
        return false;
    }
    if (width <= 0 || height <= 0)
    {
        return true;
    }
    SDL_Rect box = {0};
    if (!SotnRender_FitImage(width, height, presenter->width, presenter->height, &box))
        return false;
    SotnGlApi *gl = &presenter->gl;
    gl->ClearColor(17.0f / 255.0f, 22.0f / 255.0f, 38.0f / 255.0f, 1.0f);
    gl->Clear(GL_COLOR_BUFFER_BIT);
    gl->Viewport(box.x, box.y, box.w, box.h);
    gl->UseProgram(presenter->program);
    gl->ActiveTexture(GL_TEXTURE0);
    gl->BindTexture(GL_TEXTURE_2D, presenter->texture);
    gl->BindVertexArray(presenter->vertex_array);
    gl->DrawArrays(GL_TRIANGLES, 0, 6);
    if (!CheckGl(presenter, "Cannot draw presentation image"))
    {
        return false;
    }
    return !swap || SDL_GL_SwapWindow(presenter->window);
}

bool SotnPresenter_Draw(SotnPresenter *presenter)
{
    return DrawFrame(presenter, true);
}

void SotnPresenter_Destroy(SotnPresenter *presenter)
{
    if (presenter != NULL)
    {
        if (presenter->context != NULL)
        {
            if (SDL_GL_MakeCurrent(presenter->window, presenter->context))
            {
                if (presenter->texture != 0)
                    presenter->gl.DeleteTextures(1, &presenter->texture);
                if (presenter->vertices != 0)
                    presenter->gl.DeleteBuffers(1, &presenter->vertices);
                if (presenter->vertex_array != 0)
                    presenter->gl.DeleteVertexArrays(1, &presenter->vertex_array);
                if (presenter->program != 0)
                    presenter->gl.DeleteProgram(presenter->program);
            }
            SDL_GL_DestroyContext(presenter->context);
        }
        SDL_DestroyWindow(presenter->window);
        SDL_free(presenter);
    }
}

static bool CheckPixel(SotnPresenter *presenter, int x, int y, const Uint8 expected[4])
{
    Uint8 actual[4] = {0};
    presenter->gl.ReadPixels(x, y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, actual);
    if (!CheckGl(presenter, "Cannot read presentation test pixel"))
        return false;
    for (size_t i = 0; i < 4; ++i)
    {
        if (SDL_abs((int)actual[i] - (int)expected[i]) > 1)
            return SDL_SetError("Framebuffer mismatch at (%d,%d), channel %zu: %u != %u", x, y, i,
                                (unsigned int)actual[i], (unsigned int)expected[i]);
    }
    return true;
}

bool SotnPresenter_SelfTest(void)
{
    static const Uint8 pixels[] = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 0, 0, 0, 0};
    static const Uint8 background[] = {17, 22, 38, 255};
    for (int pass = 0; pass < 2; ++pass)
    {
        SotnPresenter *presenter = NULL;
        if (!SotnPresenter_Create("SOTN presentation test", 96, 64, &presenter))
            return false;
        const bool rejected = !SotnPresenter_SetImage(presenter, pixels, 1, 2, 2);
        bool valid = rejected && SotnPresenter_SetImage(presenter, pixels, sizeof(pixels), 2, 2) &&
                     !SotnPresenter_SetImage(presenter, pixels, 1, 2, 2) &&
                     DrawFrame(presenter, false);
        int width = 0, height = 0;
        if (valid)
            valid = SDL_GetWindowSizeInPixels(presenter->window, &width, &height);
        if (valid)
        {
            SDL_Rect box = {0};
            valid = SotnRender_FitImage(width, height, 2, 2, &box);
            valid =
                valid && CheckPixel(presenter, box.x + box.w / 4, box.y + box.h * 3 / 4, pixels) &&
                CheckPixel(presenter, box.x + box.w * 3 / 4, box.y + box.h * 3 / 4, pixels + 4) &&
                CheckPixel(presenter, box.x + box.w / 4, box.y + box.h / 4, pixels + 8) &&
                CheckPixel(presenter, box.x + box.w * 3 / 4, box.y + box.h / 4, background) &&
                CheckPixel(presenter, 0, 0, background);
        }
        char detail[1024] = {0};
        SDL_strlcpy(detail, SDL_GetError(), sizeof(detail));
        SotnPresenter_Destroy(presenter);
        if (!valid)
            return SDL_SetError("OpenGL self-test failed: %s", detail);
    }
    return true;
}
