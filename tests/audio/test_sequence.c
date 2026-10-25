#include <SDL3/SDL.h>
#include <sotn/sequence.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
typedef struct Sink
{
    SotnSequenceEvent event[32];
    Uint32 count;
    bool reject;
} Sink;
static bool Record(void *ctx, SotnSequenceEvent event)
{
    Sink *s = ctx;
    if (s->reject)
        return SDL_SetError("receiver failed");
    if (s->count >= SDL_arraysize(s->event))
        return SDL_SetError("test receiver overflow");
    s->event[s->count++] = event;
    return true;
}
static void Header(Uint8 *b, Uint16 resolution, Uint32 tempo)
{
    SDL_memcpy(b, "pQES", 4);
    b[7] = 1;
    b[8] = (Uint8)(resolution >> 8);
    b[9] = (Uint8)resolution;
    b[10] = (Uint8)(tempo >> 16);
    b[11] = (Uint8)(tempo >> 8);
    b[12] = (Uint8)tempo;
    b[13] = 4;
    b[14] = 2;
}
static bool Basic(void)
{
    Uint8 b[64] = {0};
    Header(b, 60, 500000);
    const Uint8 program[] = {0,    0xc2, 5,   0, 0xb2, 7, 90, 0,    10,   16, 0,
                             0x92, 60,   100, 1, 60,   0, 1,  0xff, 0x2f, 0};
    SDL_memcpy(b + 15, program, sizeof(program));
    SotnSequence s = {0};
    Sink sink = {0};
    CHECK(SotnSequence_Open((SotnDataView){b, 15 + sizeof(program)}, 3, 1, &s));
    CHECK(s.step == 20 && s.period == -1 && s.program[2] == 2 && s.pan[2] == 64);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && sink.count == 4);
    CHECK(sink.event[0].kind == SOTN_SEQUENCE_VOLUME && sink.event[0].value == 90);
    CHECK(sink.event[1].kind == SOTN_SEQUENCE_PAN && sink.event[1].value == 16);
    CHECK(sink.event[2].kind == SOTN_SEQUENCE_NOTE && sink.event[2].note == 60 &&
          sink.event[2].value == 100 && sink.event[2].channel == 2 && sink.event[2].program == 5 &&
          sink.event[2].volume == 90 && sink.event[2].pan == 16);
    CHECK(sink.event[3].value == 0 && s.delta == 0);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && !s.playing && sink.count == 5 &&
          sink.event[4].kind == SOTN_SEQUENCE_STOP);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && sink.count == 5);
    return true;
}
static bool Loops(void)
{
    Uint8 b[64] = {0};
    Header(b, 60, 500000);
    const Uint8 program[] = {0, 0xb0, 99, 20, 0,    6,  2,  0, 0x90, 60,   100,
                             1, 60,   0,  1,  0xb0, 99, 30, 0, 0xff, 0x2f, 0};
    SDL_memcpy(b + 15, program, sizeof(program));
    SotnSequence s = {0};
    Sink sink = {0};
    CHECK(SotnSequence_Open((SotnDataView){b, 15 + sizeof(program)}, 3, 1, &s));
    for (Uint16 i = 0; i < 6 && s.playing; ++i)
        CHECK(SotnSequence_Tick(&s, Record, &sink));
    CHECK(!s.playing && sink.count == 5 && s.completed == 1 && sink.event[2].value == 100 &&
          sink.event[3].value == 0);
    Header(b, 60, 500000);
    const Uint8 infinite[] = {0, 0x90, 60, 100, 1, 60, 0, 1, 0xff, 0x2f, 0};
    SDL_memcpy(b + 15, infinite, sizeof(infinite));
    sink = (Sink){0};
    CHECK(SotnSequence_Open((SotnDataView){b, 15 + sizeof(infinite)}, 3, 0, &s));
    for (Uint16 i = 0; i < 5; ++i)
        CHECK(SotnSequence_Tick(&s, Record, &sink));
    CHECK(s.playing && s.completed >= 2 && sink.count >= 6);
    return true;
}
static bool Timing(void)
{
    Uint8 b[64] = {0};
    Header(b, 1, 200000);
    const Uint8 program[] = {13, 0x90, 60, 100, 1, 0xff, 0x2f, 0};
    SDL_memcpy(b + 15, program, sizeof(program));
    SotnSequence s = {0};
    Sink sink = {0};
    CHECK(SotnSequence_Open((SotnDataView){b, 15 + sizeof(program)}, 3, 1, &s) && s.step == 120 &&
          s.period == 120);
    for (Uint16 i = 0; i < 120; ++i)
        CHECK(SotnSequence_Tick(&s, Record, &sink));
    CHECK(sink.count == 0 && s.period == 0 && s.delta == 130);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && s.period == 120 && s.delta == 129);
    for (Uint16 i = 0; i < 1089; ++i)
        CHECK(SotnSequence_Tick(&s, Record, &sink));
    CHECK(sink.count == 0 && s.delta == 120);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && sink.count == 1 && s.playing);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && sink.count == 2 && !s.playing);
    // Meta tempo has no MIDI length byte and uses floor BPM, unlike initial rounding.
    Header(b, 600, 501000);
    const Uint8 change[] = {0, 0xff, 0x51, 7, 0xa5, 8, 0, 0xe2, 0, 80, 30, 0xff, 0x2f, 0};
    SDL_memcpy(b + 15, change, sizeof(change));
    sink = (Sink){0};
    CHECK(SotnSequence_Open((SotnDataView){b, 15 + sizeof(change)}, 3, 1, &s));
    CHECK(s.step == 200);
    CHECK(SotnSequence_Tick(&s, Record, &sink) && sink.count == 1 &&
          sink.event[0].kind == SOTN_SEQUENCE_BEND && sink.event[0].value == 80 && s.step == 198);
    return true;
}
static bool Invalid(void)
{
    Uint8 b[48] = {0};
    Header(b, 60, 500000);
    SotnSequence s = {.cursor = 123};
    Sink sink = {0};
    CHECK(!SotnSequence_Open((SotnDataView){b, 15}, 3, 1, &s) && s.cursor == 123);
    b[7] = 2;
    CHECK(!SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s) && s.cursor == 123);
    b[7] = 1;
    Header(b, 0, 500000);
    CHECK(!SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s));
    Header(b, 60, 0);
    CHECK(!SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s));
    Header(b, 60, 500000);
    b[15] = b[16] = b[17] = b[18] = 128;
    CHECK(!SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s));
    b[15] = 0;
    b[16] = 60;
    CHECK(SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s));
    CHECK(!SotnSequence_Tick(&s, Record, &sink));
    b[16] = 0x90;
    b[17] = 60;
    b[18] = 128;
    CHECK(SotnSequence_Open((SotnDataView){b, sizeof(b)}, 3, 1, &s));
    CHECK(!SotnSequence_Tick(&s, Record, &sink));
    b[18] = 100;
    b[19] = 0;
    CHECK(SotnSequence_Open((SotnDataView){b, 20}, 3, 1, &s));
    sink.reject = true;
    CHECK(!SotnSequence_Tick(&s, Record, &sink) &&
          SDL_strcmp(SDL_GetError(), "receiver failed") == 0);
    // An immediate infinite EOF cannot monopolize an audio update.
    b[16] = 0xff;
    b[17] = 0x2f;
    CHECK(SotnSequence_Open((SotnDataView){b, 18}, 3, 0, &s));
    sink.reject = false;
    CHECK(!SotnSequence_Tick(&s, Record, &sink));
    CHECK(!SotnSequence_Tick(NULL, Record, &sink));
    return true;
}
int main(void)
{
    return Basic() && Loops() && Timing() && Invalid() ? 0 : 1;
}
