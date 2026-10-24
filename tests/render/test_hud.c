#include <SDL3/SDL.h>
#include <sotn/hud.h>

#define CHECK(value)                                                                               \
    do                                                                                             \
    {                                                                                              \
        if (!(value))                                                                              \
        {                                                                                          \
            SDL_Log("Line %d: %s: %s", __LINE__, #value, SDL_GetError());                          \
            return false;                                                                          \
        }                                                                                          \
    } while (0)

typedef struct Fixture
{
    Uint8 dra[0x26000], common[0x42000];
    Uint16 frame[SOTN_FRAME_PIXELS];
} Fixture;

static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}

static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint32 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}

static void MakeFixture(Fixture *f)
{
    SDL_memset(f, 0, sizeof(*f));
    Uint8 *layout = f->dra + 0x2ee8;
    for (Uint32 i = 0; i < 14; ++i)
    {
        layout[i] = 120;
        layout[16 + i] = (Uint8)(i < 10 ? 5 : 24);
        layout[64 + i] = 8;
        layout[80 + i] = (Uint8)(i >= 6 && i < 10 ? 16 : 8);
        Put16(layout + 96 + i * 2, (Uint16)(i < 10 ? 0x171 : 0x196));
        Put16(layout + 124 + i * 2, i < 6 ? 8 : 0);
        Put16(layout + 152 + i * 2, 0x175);
    }
    layout[5] = 30;
    layout[16 + 5] = 15;
    layout[32 + 5] = 112;
    layout[80 + 5] = 56;
    Put16(layout + 124 + 5 * 2, 0);
    for (Uint32 palette = 0; palette < 256; ++palette)
        for (Uint32 index = 1; index < 16; ++index)
            Put16(f->common + 0x40000 + (palette * 16 + index) * 2, (Uint16)(100 + index));
    for (Uint32 digit = 0; digit < 10; ++digit)
        for (Uint32 y = 0; y < 16; ++y)
            for (Uint32 word = 0; word < 2; ++word)
            {
                const Uint32 u = 32 + digit * 8 + word * 4;
                Put16(f->common + (28 + (u / 4) / 32) * 8192 + y * 64 + ((u / 4) % 32) * 2,
                      (Uint16)((y == 1 ? 14U : digit + 1) * 0x1111));
            }
    for (Uint32 i = 0; i < 11; ++i)
    {
        const size_t offset = 0x6000 + i * 256;
        Put32(f->dra + 0x252f8 + i * 4, (Uint32)(0x800a0000 + offset));
        f->dra[offset] = (Uint8)(i == 10 ? 8 : 16);
        f->dra[offset + 1] = (Uint8)(i == 10 ? 56 : 16);
        SDL_memset(f->dra + offset + 4, 0x11, i == 10 ? 224 : 128);
    }
    for (Uint32 i = 0; i < 3; ++i)
    {
        Uint8 *record = f->dra + 0x4000 + i * 16;
        Put32(f->dra + 0x3bb8 + (i + 1) * 4, (Uint32)(0x800a4000 + i * 16));
        Put32(record, 1);
        Put32(record + 4, 0x171e);
        Put32(record + 8, 2);
        Put32(record + 12, (Uint32)(0x800a4100 + i * 4));
        Put16(f->dra + 0x4100 + i * 4, (Uint16)(400 + i * 100));
    }
    Put32(f->dra + 0x3bb8 + 5 * 4, 0x800a4200);
    Put32(f->dra + 0x4200, 0x40003);
    Put32(f->dra + 0x4204, 0x1600);
    Put32(f->dra + 0x4208, 256);
    for (Uint32 i = 0; i < 5; ++i)
    {
        Put32(f->dra + 0x420c + i * 4, (Uint32)(0x800a8000 + i * 512));
        Put16(f->dra + 0x8000 + i * 512 + 2 * 16 * 2 + 2, (Uint16)(200 + i));
    }
    Put32(f->dra + 0x4220, SDL_MAX_UINT32);
    Put32(f->dra + 0x3bb8 + 4 * 4, 0x800a4300);
    Put32(f->dra + 0x4300, 0x40003);
    Put32(f->dra + 0x4304, 0x1700);
    Put32(f->dra + 0x4308, 16);
    Put32(f->dra + 0x430c, 0x800a4400);
    Put32(f->dra + 0x4310, 0x800a4420);
    Put32(f->dra + 0x4314, SDL_MAX_UINT32);
}

static bool CountersAndGauge(Fixture *f)
{
    MakeFixture(f);
    SotnHud *hud = NULL;
    SotnHudStatus status = {70, 70, 20, 20, 10, 0};
    CHECK(SotnHud_Create((SotnDataView){f->dra, sizeof(f->dra)},
                         (SotnDataView){f->common, sizeof(f->common)}, status, &hud));
    CHECK(SotnHud_Draw(hud, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[5 * 256 + 8] == 108 && f->frame[5 * 256 + 15] == 101);
    CHECK(f->frame[6 * 256 + 8] == 500);
    CHECK(f->frame[15 * 256 + 30] == 200 && f->frame[15 * 256 + 79] == 200 &&
          f->frame[15 * 256 + 80] == 0);
    status.hp = 60;
    status.mp = 10;
    CHECK(SotnHud_Tick(hud, status) && SotnHud_DisplayedHp(hud) == 69);
    SDL_memset(f->frame, 0, sizeof(f->frame));
    CHECK(SotnHud_Draw(hud, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[5 * 256 + 8] == 107 && f->frame[5 * 256 + 15] == 110);
    CHECK(f->frame[6 * 256 + 8] == 400);
    CHECK(f->frame[15 * 256 + 54] == 101 && f->frame[15 * 256 + 55] == 0);
    CHECK(SotnHud_Draw(hud, f->frame, SOTN_FRAME_PIXELS) && SotnHud_DisplayedHp(hud) == 69);
    status.hp = 0;
    status.hearts = 9999;
    for (Uint32 tick = 0; tick < 150; ++tick)
        CHECK(SotnHud_Tick(hud, status));
    CHECK(SotnHud_DisplayedHp(hud) == 0);
    CHECK(SotnHud_Draw(hud, f->frame, SOTN_FRAME_PIXELS));
    CHECK(f->frame[6 * 256 + 15] == 600);
    CHECK(f->frame[24 * 256 + 59] == 110 && f->frame[24 * 256 + 71] == 110);
    status.subweapon = 9;
    for (Uint32 tick = 0; tick < 1000; ++tick)
        CHECK(SotnHud_Tick(hud, status) && SotnHud_Draw(hud, f->frame, SOTN_FRAME_PIXELS));
    status.mp_max = 0;
    CHECK(!SotnHud_Tick(hud, status) && SotnHud_DisplayedHp(hud) == 0);
    CHECK(!SotnHud_Draw(hud, f->frame, 1));
    SotnHud_Destroy(hud);
    return true;
}

static bool Failures(Fixture *f)
{
    MakeFixture(f);
    SotnHud *hud = NULL;
    const SotnDataView dra = {f->dra, sizeof(f->dra)}, common = {f->common, sizeof(f->common)};
    const SotnHudStatus status = {70, 70, 20, 20, 10, 0};
    CHECK(!SotnHud_Create(dra, common, (SotnHudStatus){0}, &hud) && hud == NULL);
    Put32(f->dra + 0x252f8, 0xfffffffc);
    CHECK(!SotnHud_Create(dra, common, status, &hud) && hud == NULL);
    MakeFixture(f);
    Put16(f->dra + 0x2f80, 512);
    CHECK(!SotnHud_Create(dra, common, status, &hud) && hud == NULL);
    CHECK(!SotnHud_Tick(NULL, status));
    CHECK(!SotnHud_Draw(NULL, f->frame, SOTN_FRAME_PIXELS));
    SotnHud_Destroy(NULL);
    return true;
}

int main(void)
{
    Fixture *fixture = SDL_calloc(1, sizeof(*fixture));
    if (fixture == NULL)
        return 1;
    const bool passed = CountersAndGauge(fixture) && Failures(fixture);
    SDL_free(fixture);
    if (!passed)
        return 1;
    SDL_Log("SOTN_HUD_TEST: PASS");
    return 0;
}
