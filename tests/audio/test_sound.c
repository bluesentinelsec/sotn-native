#include "sound_internal.h"
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

static bool VolumePan(Fixture *f)
{
    SotnSoundSources sources = MakeFixture(f);
    Put16(f->dra, 0x1d17c, 0);
    Put16(f->dra, 0x1d17e, 127);
    Put16(f->dra, 0x1d1bc, 127);
    Put16(f->dra, 0x1d1be, 0);
    for (Uint16 id = 0x601; id <= 0x602; ++id)
        for (Sint8 pan = -8; pan <= 8; pan = (Sint8)(pan + 8))
        {
            SotnSound *sound = NULL;
            CHECK(SotnSound_Create(sources, &sound));
            CHECK(!SotnSound_QueueVolumePan(sound, 0x601, 128, pan));
            CHECK(!SotnSound_QueueVolumePan(sound, 0x601, 64, 9));
            CHECK(!SotnSound_QueueVolumePan(sound, 0xa1, 64, pan));
            CHECK(SotnSound_QueueVolumePan(sound, id, 64, pan));
            Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
            CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
            Uint32 left = 0, right = 0;
            for (Uint32 i = 0; i < SOTN_SOUND_FRAMES; ++i)
            {
                left += (Uint32)SDL_abs(pcm[i * 2]);
                right += (Uint32)SDL_abs(pcm[i * 2 + 1]);
            }
            CHECK(pan < 0   ? left > 0 && right == 0
                  : pan > 0 ? right > 0 && left == 0
                            : left == right && left > 0);
            SotnSound_Destroy(sound);
        }
    SotnSound *sound = NULL;
    CHECK(SotnSound_Create(sources, &sound));
    CHECK(SotnSound_QueueVolumePan(sound, 0x601, 0, 8));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    for (Uint32 i = 0; i < SDL_arraysize(pcm); ++i)
        CHECK(pcm[i] == 0);
    Put16(f->dra, 0x1d1bc, 128);
    CHECK(SotnSound_QueueVolumePan(sound, 0x601, 64, 8));
    CHECK(!SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    SotnSound_Destroy(sound);
    return true;
}

static bool MovingAmbience(Fixture *f)
{
    const SotnSoundSources sources = MakeFixture(f);
    f->dra[0x1f554 + 7 + 4] = 1;
    Put16(f->dra, 0x1d19c, 127);
    Put16(f->dra, 0x1d19e, 127);
    Put16(f->dra, 0x1d17c, 0);
    Put16(f->dra, 0x1d17e, 127);
    SotnSound *a = NULL, *b = NULL;
    CHECK(SotnSound_Create(sources, &a) && SotnSound_Create(sources, &b));
    CHECK(!SotnSound_QueueAmbienceVolume(a, 128, 0));
    CHECK(!SotnSound_QueueAmbienceVolume(a, 127, -9));
    CHECK(!SotnSound_QueueAmbienceVolume(NULL, 127, 0));
    CHECK(SotnSound_QueueAmbienceVolume(a, 127, 0));
    CHECK(SotnSound_Queue(a, 0x601) && SotnSound_Queue(b, 0x601));
    Sint16 one[SOTN_SOUND_FRAMES * 2] = {0}, two[SOTN_SOUND_FRAMES * 2] = {0};
    for (Uint32 tick = 0; tick < 3; ++tick)
        CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)) &&
              SotnSound_Tick(b, two, SDL_arraysize(two)) && SDL_memcmp(one, two, sizeof(one)) == 0);
    // Same gain is sample-identical to uninterrupted playback, including ADSR and loop phase.
    CHECK(SotnSound_QueueAmbienceVolume(a, 127, 0));
    CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)) &&
          SotnSound_Tick(b, two, SDL_arraysize(two)) && SDL_memcmp(one, two, sizeof(one)) == 0);
    CHECK(SotnSound_QueueAmbienceVolume(a, 0, 0));
    CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)) && SotnSound_Tick(b, two, SDL_arraysize(two)));
    for (Uint32 i = 0; i < SDL_arraysize(one); ++i)
        CHECK(one[i] == 0);
    CHECK(SotnSound_QueueAmbienceVolume(a, 127, 0));
    CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)) &&
          SotnSound_Tick(b, two, SDL_arraysize(two)) && SDL_memcmp(one, two, sizeof(one)) == 0);
    CHECK(SotnSound_QueueAmbienceVolume(a, 127, -8));
    CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)));
    Uint32 audible = 0;
    for (Uint32 i = 0; i < SOTN_SOUND_FRAMES; ++i)
    {
        CHECK(one[i * 2 + 1] == 0);
        audible += one[i * 2] != 0;
    }
    CHECK(audible > 100);
    for (Uint32 i = 0; i < 256; ++i)
        CHECK(SotnSound_QueueAmbienceVolume(a, 64, 0));
    CHECK(!SotnSound_QueueAmbienceVolume(a, 64, 0));
    CHECK(SotnSound_Tick(a, one, SDL_arraysize(one)));
    SotnSound_Destroy(a);
    SotnSound_Destroy(b);
    return true;
}

static bool ScriptPauseState(void)
{
    SotnSound *s = SDL_calloc(1, sizeof(*s));
    CHECK(s != NULL);
    for (Uint16 i = 0; i < 4; ++i)
        s->scripts[i] = (SoundScript){.id = (Uint16)(10U + i),
                                      .allocation_id = (Uint16)(30U + i),
                                      .address = 0x800a1000U + i * 6U,
                                      .timer = (Uint8)(7U + i),
                                      .steps = (Uint16)(100U + i),
                                      .volume = (Uint8)(80U + i),
                                      .pan = (Sint8)((Sint32)i - 2),
                                      .priority = (Uint8)(5U + i),
                                      .mode = i == 1 ? 5 : 0};
    s->voices[0].active = true;
    s->voices[0].env_vol = 1000;
    s->registers[0].left = 500;
    const SotnVoice voice = s->voices[0];
    const SotnVoiceRegisters registers = s->registers[0];
    const SoundScript continued = s->scripts[1], held = s->scripts[2];
    SoundPauseScripts(s);
    CHECK(s->scripts_paused && !s->scripts[0].id && !s->scripts[2].id && !s->scripts[3].id &&
          !SDL_memcmp(&s->scripts[1], &continued, sizeof(continued)) &&
          s->script_backup[2].id == held.id && s->script_backup[2].timer == held.timer &&
          !s->script_backup[2].allocation_id && !s->script_backup[2].priority);
    SoundScript copies[4] = {0};
    SDL_memcpy(copies, s->script_backup, sizeof(copies));
    SoundPauseScripts(s);
    CHECK(!SDL_memcmp(copies, s->script_backup, sizeof(copies)) &&
          !SDL_memcmp(&voice, &s->voices[0], sizeof(voice)) &&
          !SDL_memcmp(&registers, &s->registers[0], sizeof(registers)));
    SoundResumeScripts(s);
    CHECK(!s->scripts_paused && s->scripts[0].id == held.id &&
          s->scripts[0].address == held.address && s->scripts[0].steps == held.steps &&
          s->scripts[0].timer == held.timer && !s->scripts[0].allocation_id &&
          !s->scripts[0].priority && !s->scripts[2].id && s->scripts[3].id == 13 &&
          !s->script_backup[0].id && !s->script_backup[2].id && !s->script_backup[3].id &&
          !SDL_memcmp(&s->scripts[1], &continued, sizeof(continued)));
    for (Uint16 i = 0; i < 4; ++i)
    {
        s->scripts[i] = (SoundScript){.id = (Uint16)(50U + i), .allocation_id = (Uint16)(50U + i)};
        s->script_backup[i] = (SoundScript){.id = (Uint16)(70U + i)};
    }
    SoundResumeScripts(s);
    CHECK(!s->script_backup[0].id && !s->script_backup[1].id && !s->script_backup[2].id &&
          s->script_backup[3].id == 73 && s->scripts[0].id == 50 && s->scripts[3].id == 53);
    s->scripts[3].allocation_id = 0;
    SoundResumeScripts(s);
    CHECK(!s->script_backup[3].id && s->scripts[3].id == 73 && !s->scripts[3].allocation_id);
    SDL_free(s);
    return true;
}
static bool ScriptPauseQueue(Fixture *f)
{
    const SotnSoundSources source = MakeFixture(f);
    const size_t event = 0x1f554 + 3U * 7U;
    f->dra[event] = 9;
    f->dra[event + 1] = 1;
    f->dra[event + 3] = 127;
    f->dra[event + 4] = 5;
    Put32(f->dra, 0x21ed4, 0x800a1010);
    const Uint8 stream[] = {0, 0, 60, 0, 127, 2, 0, 0, 61, 0, 127, 2, 255};
    SDL_memcpy(f->dra + 0x1010, stream, sizeof(stream));
    SDL_memcpy(f->dra + event + 7, f->dra + event, 7);
    f->dra[event + 7 + 4] = 0;
    SDL_memcpy(f->dra + event + 14, f->dra + event + 7, 7);
    SotnSound *s = NULL;
    CHECK(SotnSound_Create(source, &s));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_Queue(s, 0x602) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->scripts[0].id == 0x602 && s->scripts[0].allocation_id == 0x602 &&
          s->scripts[0].timer == 2);
    CHECK(SotnSound_Queue(s, 0x603) && SotnSound_Queue(s, 0xe) &&
          SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(s->scripts_paused && !s->scripts[0].id && s->script_backup[0].id == 0x602 &&
          s->script_backup[0].timer == 2 && s->scripts[1].id == 0x603 && s->scripts[1].timer == 2);
    const SoundScript held = s->script_backup[0];
    CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          !SDL_memcmp(&held, &s->script_backup[0], sizeof(held)) && s->scripts[1].timer == 1);
    CHECK(SotnSound_Queue(s, 0x604) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[0].id == 0x604 && s->scripts[0].allocation_id == 0x604 &&
          s->scripts[1].id == 0x603 && s->script_backup[0].timer == 2);
    CHECK(SotnSound_Queue(s, 0xf) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(!s->scripts_paused && s->scripts[2].id == 0x602 && s->scripts[2].timer == 1 &&
          !s->scripts[2].allocation_id && !s->script_backup[0].id);
    CHECK(SotnSound_Queue(s, 0x605) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[2].id == 0x605 && s->scripts[2].allocation_id == 0x605);
    CHECK(!SotnSound_QueueVolumePan(s, 0xe, 127, 0) && !SotnSound_QueueVolumePan(s, 0xf, 127, 0));
    SotnSound_Destroy(s);
    CHECK(SotnSound_Create(source, &s));
    CHECK(SotnSound_Queue(s, 0x602));
    for (Uint16 i = 0; i < 4; ++i)
        CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)));
    CHECK(!s->scripts[0].id && s->scripts[0].allocation_id == 0x602);
    CHECK(SotnSound_Queue(s, 0x603) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[1].id == 0x603);
    for (Uint16 i = 0; i < 4; ++i)
    {
        s->voices[i].env_state = ADSR_OFF;
        s->voices[i].env_vol = 0;
    }
    CHECK(SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) && !s->scripts[0].allocation_id);
    SotnSound_Destroy(s);
    CHECK(SotnSound_Create(source, &s));
    /* Retained tail leases: strict priority wins before an earlier equal priority. */
    f->dra[0x1f554 + 2U * 7U + 6] = 2;
    s->scripts[0] = (SoundScript){.allocation_id = 1, .priority = 2};
    s->scripts[1] = (SoundScript){.allocation_id = 2, .priority = 1};
    s->scripts[2] = (SoundScript){.allocation_id = 3, .priority = 2};
    CHECK(SotnSound_Queue(s, 0x602) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[1].id == 0x602 && !s->scripts[0].id && !s->scripts[2].id);
    SotnSound_Destroy(s);
    CHECK(SotnSound_Create(source, &s));
    /* Authored engine selection fixture; no SEQ/hardware parity is implied. */
    s->sequence.playing = true;
    CHECK(SotnSound_Queue(s, 0x602) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[3].id == 0x602 && !s->scripts[0].id &&
          (s->voices[14].active || s->voices[16].active));
    CHECK(SotnSound_Queue(s, 0xe) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          !s->scripts[3].id && s->script_backup[3].id == 0x602);
    CHECK(SotnSound_Queue(s, 0xf) && SotnSound_Tick(s, pcm, SDL_arraysize(pcm)) &&
          s->scripts[3].id == 0x602 && !s->script_backup[3].id);
    SotnSound_Destroy(s);
    return true;
}
static bool FamiliarBank(Fixture *f)
{
    const SotnSoundSources sources = MakeFixture(f);
    Put32(f->dra, 0x1d1d0, 0x4000);
    SotnSound *sound = NULL, *before = SDL_malloc(sizeof(*before));
    CHECK(before && SotnSound_Create(sources, &sound) && SotnSound_Queue(sound, 0x601));
    Sint16 pcm[SOTN_SOUND_FRAMES * 2] = {0};
    CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)));
    *before = *sound;
    SotnSound *candidate = NULL;
    CHECK(SotnSound_PrepareFamiliarBank(sound, sources.player_header, sources.player_body,
                                        &candidate) &&
          candidate && !SDL_memcmp(sound, before, sizeof(*sound)) && candidate->banks[2].loaded &&
          !sound->banks[2].loaded);
    Sint16 candidate_pcm[SOTN_SOUND_FRAMES * 2] = {0};
    for (Uint16 tick = 0; tick < 8; ++tick)
        CHECK(SotnSound_Tick(sound, pcm, SDL_arraysize(pcm)) &&
              SotnSound_Tick(candidate, candidate_pcm, SDL_arraysize(candidate_pcm)) &&
              !SDL_memcmp(pcm, candidate_pcm, sizeof(pcm)));
    SotnSound_Destroy(sound);
    sound = candidate;
    candidate = NULL;
    *before = *sound;
    CHECK(SotnSound_SetFamiliarBank(sound, sources.player_header, sources.player_body) &&
          sound->banks[2].loaded && sound->banks[2].address == 0x4000 &&
          sound->banks[2].samples[1] == 0x4000 &&
          !SDL_memcmp(sound->ram + 0x4000, f->body, sizeof(f->body)) &&
          !SDL_memcmp(sound->ram, before->ram, 0x4000) &&
          !SDL_memcmp(sound->ram + 0x4020, before->ram + 0x4020, sizeof(sound->ram) - 0x4020) &&
          !SDL_memcmp(sound->voices, before->voices, sizeof(sound->voices)) &&
          !SDL_memcmp(sound->registers, before->registers, sizeof(sound->registers)) &&
          !SDL_memcmp(sound->scripts, before->scripts, sizeof(sound->scripts)) &&
          !SDL_memcmp(sound->queue, before->queue, sizeof(sound->queue)));
    for (Uint16 id = 0; id < 4; ++id)
        if (id != 2)
            CHECK(!SDL_memcmp(&sound->banks[id], &before->banks[id], sizeof(SoundBank)));
    Uint8 malformed[sizeof(f->header)] = {0};
    SDL_memcpy(malformed, f->header, sizeof(malformed));
    Put16(malformed, 2594, 6);
    *before = *sound;
    CHECK(!SotnSound_PrepareFamiliarBank(sound, (SotnDataView){malformed, sizeof(malformed)},
                                         sources.player_body, &candidate) &&
          !candidate && !SDL_memcmp(sound, before, sizeof(*sound)) &&
          !SotnSound_PrepareFamiliarBank(sound, sources.player_header, (SotnDataView){0},
                                         &candidate) &&
          !candidate && !SDL_memcmp(sound, before, sizeof(*sound)) &&
          !SotnSound_SetFamiliarBank(sound, (SotnDataView){malformed, sizeof(malformed)},
                                     sources.player_body) &&
          !SDL_memcmp(sound, before, sizeof(*sound)) &&
          !SotnSound_SetFamiliarBank(sound, sources.player_header, (SotnDataView){0}) &&
          !SDL_memcmp(sound, before, sizeof(*sound)));
    CHECK(SoundKey(sound, 14, 2, 0, 0, 60, 127, 64) &&
          sound->registers[14].start_addr == 0x4000 / 8 && sound->voices[14].active);
    SotnSound_Destroy(sound);
    SDL_free(before);
    return true;
}
static void ReverbCoefficients(Uint8 bytes[64])
{
    // Authored impulse profile, unrelated to the game's private Studio B coefficients.
    const Uint16 values[32] = {1, 1, 16384, 32767, 0, 0, 0,  8192, 16384, 16384, 2,
                               3, 2, 3,     2,     3, 2, 3,  4,    5,     2,     3,
                               2, 3, 4,     5,     8, 9, 12, 13,   32767, 32767};
    for (Uint32 i = 0; i < 32; ++i)
        Put16(bytes, i * 2, values[i]);
}
static bool ReverbProcessor(void)
{
    Uint8 coefficients[64];
    ReverbCoefficients(coefficients);
    const Uint32 base = SOTN_SPU_RAM_SIZE - 4096;
    Uint8 *ram = SDL_malloc(SOTN_SPU_RAM_SIZE), *before = SDL_malloc(SOTN_SPU_RAM_SIZE);
    CHECK(ram && before);
    SDL_memset(ram, 0x5a, SOTN_SPU_RAM_SIZE);
    SDL_memset(ram + base, 0, SOTN_SPU_RAM_SIZE - base);
    SotnReverb r = {0};
    CHECK(SoundReverb_Configure(&r, base, (SotnDataView){coefficients, 64}) &&
          SoundReverb_SetDepth(&r, 127, -127) && r.depth[0] == 32767 && r.depth[1] == -32767);
    CHECK(SoundReverb_SetDepth(&r, 128, -128) && r.depth[0] == -32511 && r.depth[1] == 32511);
    CHECK(SoundReverb_SetDepth(&r, 127, 127));
    const SotnReverb held = r;
    CHECK(!SoundReverb_Configure(&r, base + 1, (SotnDataView){coefficients, 64}) &&
          !SDL_memcmp(&r, &held, sizeof(r)));
    Put16(coefficients, 20, 0xffff);
    CHECK(!SoundReverb_Configure(&r, base, (SotnDataView){coefficients, 64}) &&
          !SDL_memcmp(&r, &held, sizeof(r)));
    ReverbCoefficients(coefficients);
    Uint32 audible = 0;
    for (Uint32 i = 0; i < 20000; ++i)
    {
        if (i == 1000)
            CHECK(SoundReverb_SetDepth(&r, 0, 0));
        Sint32 output[2];
        SoundReverb_Tick(&r, ram, i ? 0 : 32767, i ? 0 : -32768, output);
        if (i < 1000)
            audible += output[0] != 0 || output[1] != 0;
        else
            CHECK(!output[0] && !output[1]);
    }
    CHECK(audible && !r.phase && r.cursor == base + 20000 % 4096);
    for (Uint32 i = 0; i < base; ++i)
        CHECK(ram[i] == 0x5a);
    SDL_memcpy(before, ram, SOTN_SPU_RAM_SIZE);
    const Uint32 cursor = r.cursor;
    r.enabled = false;
    for (Uint32 i = 0; i < 16; ++i)
    {
        Sint32 output[2];
        SoundReverb_Tick(&r, ram, 32767, -32768, output);
    }
    CHECK(!SDL_memcmp(before, ram, SOTN_SPU_RAM_SIZE) && r.cursor == cursor + 16 && !r.phase);
    SDL_free(before);
    SDL_free(ram);
    return true;
}
static bool ReverbOwner(Fixture *f)
{
    SotnSoundSources sources = MakeFixture(f);
    f->header[2081] = 4;
    Put32(f->dra, 0x1d1d0, 0x4000);
    SotnSound *sound = NULL, *candidate = NULL, *held = SDL_malloc(sizeof(*held));
    Uint8 *image = SDL_calloc(1, 0x24000);
    CHECK(held && image && SotnSound_Create(sources, &sound));
    const Uint32 base = SOTN_SPU_RAM_SIZE - 4096;
    const size_t record = 0x23db8 + 3 * 68;
    Put32(image, 0x23d68 + 3 * 4, base / 8);
    ReverbCoefficients(image + record + 4);
    SDL_memset(sound->ram + base, 0xa5, 4096);
    *held = *sound;
    CHECK(!SotnSound_ConfigureReverb(sound, (SotnDataView){image, record + 67}) &&
          !SDL_memcmp(sound, held, sizeof(*held)));
    CHECK(SotnSound_ConfigureReverb(sound, (SotnDataView){image, 0x24000}) &&
          sound->reverb.configured && sound->reverb.enabled && sound->reverb.cursor == base &&
          !sound->reverb.phase && sound->reverb.depth[0] == 2580 &&
          !SDL_memcmp(sound->ram, held->ram, base));
    for (Uint32 i = base; i < SOTN_SPU_RAM_SIZE; ++i)
        CHECK(!sound->ram[i]);
    *held = *sound;
    CHECK(!SotnSound_ConfigureReverb(sound, (SotnDataView){image, 0x24000}) &&
          !SDL_memcmp(sound, held, sizeof(*held)));
    CHECK(SoundKey(sound, 14, 1, 0, 0, 60, 127, 64) && sound->reverb_voices == (1U << 14));
    *held = *sound;
    held->reverb.configured = false; // Independent dry rendering of the same actual voice state.
    Uint32 different = 0;
    for (Uint32 i = 0; i < 16; ++i)
    {
        Sint16 wet[SOTN_SOUND_FRAMES * 2], dry[SOTN_SOUND_FRAMES * 2];
        CHECK(SotnSound_Tick(sound, wet, SDL_arraysize(wet)) &&
              SotnSound_Tick(held, dry, SDL_arraysize(dry)));
        for (Uint32 j = 0; j < SDL_arraysize(wet); ++j)
            different += wet[j] != dry[j];
    }
    CHECK(different && sound->reverb.cursor == base + (16 * SOTN_SOUND_FRAMES) % 4096 &&
          !sound->reverb.phase &&
          SotnSound_PrepareFamiliarBank(sound, sources.player_header, sources.player_body,
                                        &candidate) &&
          !SDL_memcmp(&sound->reverb, &candidate->reverb, sizeof(sound->reverb)) &&
          !SDL_memcmp(sound->ram + base, candidate->ram + base, 4096));
    SotnSound_Destroy(candidate);
    candidate = NULL;
    Put32(f->dra, 0x1d1d0, base);
    *held = *sound;
    CHECK(!SotnSound_PrepareFamiliarBank(sound, sources.player_header, sources.player_body,
                                         &candidate) &&
          !candidate && !SDL_memcmp(sound, held, sizeof(*held)));
    CHECK(SoundKey(sound, 14, 0, 0, 0, 60, 127, 64) && !sound->reverb_voices);
    SotnSound_Destroy(sound);
    SDL_free(held);
    SDL_free(image);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (f == NULL)
        return 1;
    const bool passed = Decode() && Simulation(f) && Ambience(f) && VolumePan(f) &&
                        MovingAmbience(f) && Failures(f) && ScriptPauseState() &&
                        ScriptPauseQueue(f) && FamiliarBank(f) && ReverbProcessor() &&
                        ReverbOwner(f);
    SDL_free(f);
    if (!passed)
        return 1;
    SDL_Log("SOTN_AUDIO_TEST: PASS");
    return 0;
}
