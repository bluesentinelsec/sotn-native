#include "sound_internal.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Fixture
{
    Uint8 dra[0x23000], exe[0x23896], header[3104], body[32], seq[64];
} Fixture;
static void Put16(Uint8 *p, size_t o, Uint16 v)
{
    p[o] = (Uint8)v;
    p[o + 1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, size_t o, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[o + i] = (Uint8)(v >> (i * 8U));
}
static bool Open(Fixture *f, SotnSound **out)
{
    SDL_zero(*f);
    Put32(f->header, 0, 0x56414270);
    Put32(f->header, 4, 7);
    Put16(f->header, 18, 1);
    Put16(f->header, 22, 1);
    f->header[24] = 127;
    f->header[25] = 64;
    f->header[32] = 1;
    f->header[33] = 127;
    f->header[36] = 64;
    f->header[2080] = 100;
    f->header[2082] = 127;
    f->header[2083] = 64;
    f->header[2084] = 60;
    f->header[2087] = 127;
    f->header[2092] = f->header[2093] = 2;
    Put16(f->header, 2096, 15);
    Put16(f->header, 2098, 0x1fc0);
    Put16(f->header, 2102, 1);
    Put16(f->header, 2594, 4);
    SDL_memcpy(f->dra + 0x1107c, f->header, sizeof(f->header));
    Put32(f->dra, 0x1d1c8, 0x1000);
    Put32(f->dra, 0x1d1cc, 0x2000);
    Put32(f->dra, 0x1d1d4, 0x3000);
    f->dra[0x1d1e6] = 127;
    f->dra[0x1d1e7] = 0x30;
    f->dra[0x1d1e8] = 0;
    for (Uint16 i = 0; i < 2; ++i)
    {
        f->body[i * 16] = 4;
        f->body[i * 16 + 1] = (Uint8)(i ? 3 : 6);
        for (Uint16 j = 2; j < 16; ++j)
            f->body[i * 16 + j] = 0x11;
    }
    Put16(f->exe, 0x23714, 4096);
    Put16(f->exe, 0x23714 + 32 * 2, 5000);
    Put16(f->exe, 0x23714 + 159 * 2, 7000);
    SDL_memcpy(f->seq, "pQES", 4);
    f->seq[7] = 1;
    f->seq[9] = 60;
    f->seq[10] = 7;
    f->seq[11] = 0xa1;
    f->seq[12] = 0x20;
    f->seq[13] = 4;
    f->seq[14] = 2;
    const Uint8 program[] = {0,  0xc0, 0,    0, 0x90, 60, 100,  64, 0xb0, 10, 16,   64,   7,
                             64, 64,   0xe0, 0, 64,   64, 0x90, 60, 0,    1,  0xff, 0x2f, 0};
    SDL_memcpy(f->seq + 15, program, sizeof(program));
    const SotnSoundSources inputs = {{f->dra, sizeof(f->dra)},       {f->body, sizeof(f->body)},
                                     {f->header, sizeof(f->header)}, {f->body, sizeof(f->body)},
                                     {f->header, sizeof(f->header)}, {f->body, sizeof(f->body)}};
    return SotnSound_Create(inputs, out) &&
           SotnSound_SetSequence(*out, (SotnDataView){f->exe, sizeof(f->exe)},
                                 (SotnDataView){f->seq, 15 + sizeof(program)});
}
static bool Playback(Fixture *f)
{
    SotnSound *s = NULL;
    CHECK(Open(f, &s));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_PlaySequence(s, 0x202) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->sequence_voices[0].sequence && s->sequence_voices[0].note == 60 &&
          s->registers[0].pitch == 4096);
    CHECK(s->registers[0].left == 10157 && s->registers[0].right == 10157);
    bool audible = false;
    for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
        audible |= pcm[i] != 0;
    CHECK(audible);
    CHECK(!SotnSound_PlaySequence(s, 0x203) && s->sequence.playing);
    CHECK(!SotnSound_SetSequence(s, (SotnDataView){0}, (SotnDataView){0}) && s->sequence.playing);
    for (Uint16 i = 0; i < 64 && s->sequence_voices[0].pan == 64; ++i)
        CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->sequence_voices[0].pan == 16 && s->registers[0].left == 10157 &&
          s->registers[0].right == 655);
    for (Uint16 i = 0; i < 64 && s->sequence_voices[0].volume == 127; ++i)
        CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->sequence_voices[0].volume == 64 && s->registers[0].left == 2539 &&
          s->registers[0].right == 163);
    for (Uint16 i = 0; i < 200 && s->sequence.playing; ++i)
        CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(!s->sequence.playing && s->registers[0].pitch == 4096 && s->voices[0].key_off);
    CHECK(SotnSound_PlaySequence(s, 0x202) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(SoundKey(s, 20, 3, 0, 0, 60, 127, 64));
    SotnSound_StopSequence(s);
    CHECK(!s->sequence.playing && !s->voices[20].key_off);
    for (Uint16 i = 0; i < 16; ++i)
        if (s->sequence_voices[i].sequence)
            CHECK(s->voices[i].key_off);
    SotnSound_Destroy(s);
    return true;
}
static bool Pool(Fixture *f)
{
    SotnSound *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < 16; ++i)
    {
        s->sequence_voices[i] = (SoundSeqVoice){.claimed = true, .priority = 200};
        s->voices[i].env_vol = 123;
    }
    CHECK(SotnSound_PlaySequence(s, 0x202) && SoundSequenceTick(s));
    for (Uint16 i = 0; i < 16; ++i)
        CHECK(!s->sequence_voices[i].sequence);
    s->sequence_voices[7].priority = s->sequence_voices[9].priority = 1;
    s->sequence_voices[7].age = 9;
    s->sequence_voices[9].age = 2;
    CHECK(SotnSound_PlaySequence(s, 0x202) && SoundSequenceTick(s));
    CHECK(s->sequence_voices[7].sequence && !s->sequence_voices[9].sequence);
    // A direct effect key invalidates only the sequence note on its hardware voice.
    CHECK(SoundKey(s, 7, 3, 0, 0, 60, 127, 64) && !s->sequence_voices[7].sequence);
    SotnSound_StopSequence(s);
    CHECK(!s->voices[7].key_off);
    f->dra[0x1d1e8] = 0x20;
    CHECK(!SotnSound_PlaySequence(s, 0x202));
    SotnSound_Destroy(s);
    return true;
}
static bool Bends(Fixture *f)
{
    SotnSound *s = NULL;
    CHECK(Open(f, &s));
    SotnSound_StopSequence(s);
    const Uint8 program[] = {0, 0xc0, 0, 0, 0x90, 60, 100,  1,    0xe0,
                             0, 127,  1, 0, 0,    1,  0xff, 0x2f, 0};
    SDL_memcpy(f->seq + 15, program, sizeof(program));
    CHECK(SotnSound_SetSequence(s, (SotnDataView){f->exe, sizeof(f->exe)},
                                (SotnDataView){f->seq, 15 + sizeof(program)}));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_PlaySequence(s, 0x202) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->registers[0].pitch == 5000 && s->sequence.playing);
    CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) && s->registers[0].pitch == 3500 &&
          !s->sequence.playing);
    SotnSound_Destroy(s);
    return true;
}
static bool Retirement(Fixture *f)
{
    SotnSound *s = NULL;
    CHECK(Open(f, &s));
    for (Uint16 i = 0; i < 16; ++i)
        s->sequence_voices[i].claimed = true;
    for (Uint16 i = 0; i < 15; ++i)
        CHECK(SoundSequenceTick(s));
    CHECK(s->sequence_voices[0].claimed && s->sequence_voices[15].claimed);
    CHECK(SoundSequenceTick(s) && !s->sequence_voices[0].claimed &&
          !s->sequence_voices[15].claimed);
    // Keep the original skipped history bin even when its current envelope is nonzero.
    for (Uint16 i = 0; i < 16; ++i)
        s->sequence_silence[i] = SDL_MAX_UINT16;
    s->sequence_flush = 14;
    s->sequence_voices[7].claimed = true;
    s->voices[7].env_vol = 123;
    CHECK(SoundSequenceTick(s) && !s->sequence_voices[7].claimed);
    CHECK(!(s->sequence_silence[15] & (1U << 7)));
    SotnSound_Destroy(s);
    CHECK(Open(f, &s));
    const Uint8 program[] = {0, 0xc0, 0,    0,  0x90, 60, 100,  1,    60,
                             0, 1,    0xb0, 10, 16,   64, 0xff, 0x2f, 0};
    SDL_memcpy(f->seq + 15, program, sizeof(program));
    CHECK(SotnSound_SetSequence(s, (SotnDataView){f->exe, sizeof(f->exe)},
                                (SotnDataView){f->seq, 15 + sizeof(program)}));
    CHECK(SotnSound_PlaySequence(s, 0x202) && SoundSequenceTick(s));
    CHECK(!s->sequence_voices[0].claimed && s->sequence_voices[0].sequence && s->voices[0].key_off);
    CHECK(SoundSequenceTick(s) && s->sequence_voices[0].pan == 16 && s->registers[0].right == 655);
    SotnSound_Destroy(s);
    return true;
}
static bool StageIdentity(Fixture *f)
{
    SotnSound *s = NULL;
    CHECK(Open(f, &s));
    f->dra[0x1d1ec] = 64;
    f->dra[0x1d1ed] = 0x21;
    f->dra[0x1d1ee] = 0;
    CHECK(!SotnSound_PlaySequence(s, 0x204));
    CHECK(SotnSound_SetStageSequence(s, (SotnDataView){f->exe, sizeof(f->exe)},
                                     (SotnDataView){f->seq, sizeof(f->seq)}, 0x204));
    CHECK(SotnSound_PlaySequence(s, 0x204) && SoundSequenceTick(s));
    CHECK(s->sequence_id == 0x204 && s->sequence.playing && s->registers[0].left < 10157);
    CHECK(!SotnSound_PlaySequence(s, 0x202) && s->sequence.playing);
    CHECK(!SotnSound_SetStageSequence(s, (SotnDataView){0}, (SotnDataView){0}, 0x204) &&
          s->sequence.playing && s->sequence_id == 0x204);
    CHECK(!SotnSound_SetStageSequence(s, (SotnDataView){f->exe, sizeof(f->exe)},
                                      (SotnDataView){f->seq, sizeof(f->seq)}, 0x203));
    // A bad new owner leaves the attached chime alive.
    CHECK(s->sequence_id == 0x204 && s->sequence.playing);
    SotnSound_StopSequence(s);
    CHECK(!s->sequence.playing && SotnSound_PlaySequence(s, 0x204));
    SotnSound_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Playback(f) && Pool(f) && Bends(f) && Retirement(f) && StageIdentity(f);
    SDL_free(f);
    return ok ? 0 : 1;
}
