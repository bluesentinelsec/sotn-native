#include "voice.h"
#include <SDL3/SDL.h>
#include <sotn/sound.h>

#define CHECK(condition)                                                                           \
    do                                                                                             \
    {                                                                                              \
        if (!(condition))                                                                          \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #condition, SDL_GetError());                      \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x23000], header[3104], body[32];
} Fixture;

static void Put16(Uint8 *p, size_t o, Uint16 v)
{
    p[o] = (Uint8)v;
    p[o + 1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, size_t o, Uint32 v)
{
    for (Uint32 i = 0; i < 4; i++)
        p[o + i] = (Uint8)(v >> (i * 8));
}

static SotnSoundSources MakeFixture(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    Put32(f->header, 0, 0x56414270);
    Put32(f->header, 4, 7);
    Put16(f->header, 18, 1);
    Put16(f->header, 20, 1);
    Put16(f->header, 22, 1);
    f->header[24] = 127;
    f->header[25] = 64;
    f->header[32] = 1;
    f->header[33] = 127;
    f->header[36] = 64;
    f->header[2082] = 127;
    f->header[2083] = 64;
    f->header[2084] = 60;
    Put16(f->header, 2096, 15);
    Put16(f->header, 2098, 0x1fc0);
    Put16(f->header, 2102, 1);
    Put16(f->header, 2594, 4);
    SDL_memcpy(f->dra + 0x1107c, f->header, sizeof(f->header));
    Put32(f->dra, 0x1d1c8, 0x1000);
    Put32(f->dra, 0x1d1cc, 0x2000);
    Put32(f->dra, 0x1d1d4, 0x3000);
    f->body[0] = 4;
    f->body[1] = 6;
    f->body[16] = 4;
    f->body[17] = 3;
    for (Uint32 i = 2; i < 16; i++)
        f->body[i] = f->body[i + 16] = 0x11;
    const size_t sound = 0x1f554 + 7;
    f->dra[sound + 2] = 60;
    f->dra[sound + 3] = 127;
    f->dra[sound + 7] = 9;
    f->dra[sound + 10] = 127;
    f->dra[sound + 18] = 2;
    Put32(f->dra, 0x21ed0, 0x800a1000);
    f->dra[0x1002] = 60;
    f->dra[0x1004] = 127;
    f->dra[0x1005] = 2;
    f->dra[0x1006] = 255;
    return (SotnSoundSources){{f->dra, sizeof(f->dra)},       {f->body, sizeof(f->body)},
                              {f->header, sizeof(f->header)}, {f->body, sizeof(f->body)},
                              {f->header, sizeof(f->header)}, {f->body, sizeof(f->body)}};
}

static bool Decode(void)
{
    Uint8 block[16] = {0};
    block[1] = 1;
    for (Uint32 i = 2; i < 16; i++)
        block[i] = 0x87;
    Sint16 history = 0, previous = 0, pcm[28] = {0};
    Uint8 flags = 0;
    spu_adpcm_decode_block(block, &history, &previous, pcm, &flags);
    CHECK(flags == 1 && history == -32768 && previous == 28672);
    for (Uint32 i = 0; i < 28; i++)
        CHECK(pcm[i] == (i % 2 ? -32768 : 28672));
    block[0] = 0x1c;
    SDL_memset(block + 2, 0, 14);
    history = 1024;
    previous = 0;
    spu_adpcm_decode_block(block, &history, &previous, pcm, &flags);
    CHECK(pcm[0] == 960 && pcm[1] == 900);
    return true;
}

static bool Simulation(Fixture *f)
{
    SotnSoundSources sources = MakeFixture(f);
    SotnSound *a = NULL, *b = NULL;
    CHECK(SotnSound_Create(sources, &a) && SotnSound_Create(sources, &b));
    CHECK(SotnSound_Queue(a, 0x601) && SotnSound_Queue(b, 0x601));
    Sint16 first[SOTN_SOUND_FRAMES * 2] = {0}, second[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_Tick(a, first, SDL_arraysize(first)) &&
          SotnSound_Tick(b, second, SDL_arraysize(second)));
    CHECK(SDL_memcmp(first, second, sizeof(first)) == 0);
    Uint32 audible = 0;
    for (size_t i = 0; i < SDL_arraysize(first); i++)
        audible += first[i] != 0;
    CHECK(audible > 100);
    CHECK(SotnSound_Queue(a, 0x602));
    CHECK(SotnSound_Tick(a, first, SDL_arraysize(first)));
    CHECK(!SotnSound_Queue(a, 0x600) && !SotnSound_Queue(a, 0x8e1));
    for (Uint32 i = 0; i < 256; i++)
        CHECK(SotnSound_Queue(a, 0x601));
    CHECK(!SotnSound_Queue(a, 0x601));
    CHECK(SotnSound_Tick(a, first, SDL_arraysize(first)));
    CHECK(!SotnSound_Tick(a, first, 1));
    Put32(f->dra, 0x21ed0, 0xfffffff0);
    CHECK(SotnSound_Queue(a, 0x602));
    CHECK(!SotnSound_Tick(a, first, SDL_arraysize(first)));
    SotnSound_Destroy(a);
    SotnSound_Destroy(b);
    return true;
}

static bool Ambience(Fixture *f)
{
    const SotnSoundSources sources = MakeFixture(f);
    f->dra[0x1f554 + 7 + 4] = 3;
    SotnSound *sound = NULL;
    CHECK(SotnSound_Create(sources, &sound));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_Queue(sound, 0x601));
    CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    Uint32 audible = 0;
    for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
        audible += pcm[i] != 0;
    CHECK(audible > 100);
    CHECK(SotnSound_Queue(sound, 0xa1));
    for (Uint32 tick = 0; tick < 30; ++tick)
        CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
        CHECK(pcm[i] == 0);
    CHECK(!SotnSound_Queue(sound, 0xa2));
    f->dra[0x1f554 + 7 + 4] = 6;
    CHECK(SotnSound_Queue(sound, 0x601));
    CHECK(!SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    SotnSound_Destroy(sound);
    return true;
}

static bool Failures(Fixture *f)
{
    SotnSoundSources sources = MakeFixture(f);
    SotnSound *sound = NULL;
    sources.common_body.size = 31;
    CHECK(!SotnSound_Create(sources, &sound) && sound == NULL);
    sources = MakeFixture(f);
    f->header[32] = 17;
    CHECK(!SotnSound_Create(sources, &sound) && sound == NULL);
    CHECK(!SotnSound_Create((SotnSoundSources){0}, &sound) && sound == NULL);
    CHECK(!SotnSound_Queue(NULL, 0x601));
    SotnSound_Destroy(NULL);
    return true;
}

int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (f == NULL)
        return 1;
    const bool passed = Decode() && Simulation(f) && Ambience(f) && Failures(f);
    SDL_free(f);
    if (!passed)
        return 1;
    SDL_Log("SOTN_AUDIO_TEST: PASS");
    return 0;
}
