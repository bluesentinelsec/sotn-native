#include "actors_internal.h"
#include "character_fixture.h"
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint16 voices_seen = 0;
typedef struct Fixture
{
    Uint8 dra[0x8000], lib[0x2000], boot[0x1e400];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    voices_seen = 0;
    TestCharacter_Fill(f->dra);
    TestCharacter_Put32(f->dra + 0x98, 0x800a0200);
    for (Uint16 i = 0; i <= 1024; ++i)
        Put16(f->boot + 0x1dbcc + i * 2U, (Uint16)(i * 4U));
    TestCharacter_Put32(f->lib + 0x1344, 0x80181a00);
    TestCharacter_Put32(f->lib + 0x1348, 0x80181a08);
    const Uint8 normal[] = {3, 0, 5, 4}, clear[] = {4, 0, 5, 2, 4};
    SDL_memcpy(f->lib + 0x1a00, normal, sizeof(normal));
    SDL_memcpy(f->lib + 0x1a08, clear, sizeof(clear));
    for (Uint16 i = 0; i < 6; ++i)
        TestCharacter_Put32(f->lib + 0x1310 + i * 4U, 0x80181a10);
    f->lib[0x1a10] = 1;
    f->lib[0x1a11] = 'A';
    for (Uint16 i = 0; i < 48; ++i)
    {
        Uint8 *p = f->lib + 0x134c + i * 8U;
        Put16(p + 2, 86);
        TestCharacter_Put32(p + 4, 2);
    }
    for (Uint16 i = 0; i < 7; ++i)
    {
        Uint8 *p = f->lib + 0x14d8 + i * 8U;
        Put16(p, 4);
        Put16(p + 2, (Uint16)(63 + i));
        TestCharacter_Put32(p + 4, 9);
        f->dra[0x3400 + (63U + i) * 32U + 28] = 3;
    }
    SDL_memcpy(f->lib + 0x16a4, "Synthetic", 8);
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->lib, sizeof(f->lib)}, 2, out));
    SotnActors *s = *out;
    s->executable = (SotnDataView){f->boot, sizeof(f->boot)};
    CHECK(SotnCharacter_Create(s->dra, &s->character));
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(s->character, &state, NULL));
    state.gold = 1000;
    CHECK(SotnCharacter_Restore(s->character, &state));
    s->slots[131] = (Actor){.slot = 131, .state = {.id = 0x26, .generation = 1}};
    return true;
}
static bool Tick(SotnActors *s, Uint16 buttons)
{
    s->tapped_buttons = (Uint16)(buttons & (Uint16)~s->previous_buttons);
    s->buttons = buttons;
    s->previous_buttons = buttons;
    ActorRepeatButtons(s);
    SDL_zero(s->combat);
    CHECK(ActorLibraryShop(s, &s->slots[131]));
    Actor *a = &s->slots[132];
    if (a->state.generation)
    {
        CHECK(ActorLibraryShop(s, a));
        if (a->remove)
        {
            ActorPrimitivesClear(s, a);
            *a = (Actor){0};
        }
    }
    if (s->combat.voice_change)
    {
        if (s->combat.voice == 0x3a9)
            voices_seen |= 1;
        if (s->combat.voice == 0x3ad)
            voices_seen |= 2;
        if (s->combat.voice == 0x3b2)
            voices_seen |= 4;
    }
    return true;
}
static bool Tap(SotnActors *s, Uint16 b)
{
    return Tick(s, 0) && Tick(s, b) && Tick(s, 0);
}
static bool Wait(SotnActors *s, Uint16 slot, Uint16 step)
{
    for (Uint16 i = 0; i < 200; ++i)
    {
        if (s->slots[slot].state.step == step)
            return true;
        CHECK(Tick(s, 0));
    }
    return SDL_SetError("Shop test wait exceeded original transition bounds");
}
static bool Flow(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    CHECK(Tick(s, 0) && Tick(s, 0));
    CHECK(s->slots[131].state.step == 2 && s->shop.count == 3);
    s->death_event.flags = 0x200;
    CHECK(Wait(s, 131, 4));
    CHECK(s->slots[131].librarian.timer == 64);
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && Wait(s, 132, 3));
    CHECK(voices_seen & 1U);
    CHECK(s->shop.catalogue.count == 48 && s->shop.quantities[0] == 1);
    CHECK(Wait(s, 131, 11));
    Uint16 labels = 0;
    for (Uint16 i = 0; i < s->slots[131].state.primitive_count; ++i)
    {
        const ActorPrimitive *p = ActorPrimitiveGet(s, &s->slots[131], i);
        CHECK(p != NULL);
        labels += (Uint16)(p->state.active && p->state.textured);
    }
    CHECK(labels == 1); // The original collapsed texture crops to the selected row.
    SotnCharacterState poor = {0};
    CHECK(SotnCharacter_Get(s->character, &poor, NULL));
    poor.gold = 1;
    CHECK(SotnCharacter_Restore(s->character, &poor));
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && s->slots[132].state.step == 3);
    CHECK(SotnCharacter_Get(s->character, &poor, NULL) && poor.gold == 1 && !poor.hand_count[86]);
    poor.gold = 1000;
    CHECK(SotnCharacter_Restore(s->character, &poor));
    CHECK(Tap(s, SOTN_BUTTON_RIGHT) && s->shop.quantities[0] == 2);
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && Wait(s, 132, 3));
    SotnCharacterState state = {0};
    CHECK(SotnCharacter_Get(s->character, &state, NULL));
    CHECK(state.gold == 996 && state.hand_count[86] == 2 && s->shop.quantities[0] == 1 &&
          s->shop.catalogue.offers[0].available == 97);
    // Pages preserve the original two-stage cursor-to-bottom then seven-row scroll behavior.
    CHECK(Tap(s, 0x800) && s->shop.item_cursor == 6 && s->shop.scroll == 0);
    CHECK(Tap(s, 0x800) && s->shop.scroll == 7);
    CHECK(Tap(s, 0x400) && s->shop.item_cursor == 0 && s->shop.scroll == 7);
    CHECK(Tap(s, 0x400) && s->shop.scroll == 0);
    CHECK(Tap(s, SOTN_BUTTON_CANCEL) && Wait(s, 131, 4));
    CHECK(!s->slots[132].state.generation);
    CHECK(SotnCharacter_Collect(s->character, true, 63));
    CHECK(Tap(s, SOTN_BUTTON_DOWN) && s->shop.cursor == 1);
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && Wait(s, 132, 5));
    CHECK(s->shop.sell && s->shop.catalogue.count == 7 && s->shop.quantities[0] == 1);
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && Wait(s, 132, 5));
    CHECK(SotnCharacter_Get(s->character, &state, NULL) && state.gold == 1005 &&
          !state.body_count[63] && !s->shop.quantities[0]);
    CHECK(Tap(s, SOTN_BUTTON_JUMP)); // Sold-out sale is an error sound, never a debit.
    CHECK(s->slots[132].state.step == 5 && SotnCharacter_Get(s->character, &state, NULL) &&
          state.gold == 1005);
    CHECK(Tap(s, SOTN_BUTTON_CANCEL) && Wait(s, 131, 4));
    s->death_event.voice_playing = true;
    CHECK(Tap(s, SOTN_BUTTON_CANCEL));
    CHECK(s->combat.voice_stop && s->slots[131].state.step == 6);
    CHECK(Tick(s, 0) && s->slots[131].state.step == 6);
    s->death_event.voice_playing = false;
    CHECK(Tick(s, 0) && s->combat.voice_change && s->combat.voice == 0x3b2);
    CHECK(Wait(s, 131, 2) && (s->death_event.flags & 0x100U) && !(s->death_event.flags & 0x200U));
    CHECK(voices_seen == 7);
    s->world->game_clear = 1;
    ActorPrimitivesClear(s, &s->slots[131]);
    s->slots[131] = (Actor){.slot = 131, .state = {.id = 0x26, .generation = 2}};
    CHECK(Tick(s, 0) && s->shop.count == 4);
    s->death_event.flags = 0x200;
    CHECK(Wait(s, 131, 4));
    CHECK(Tap(s, SOTN_BUTTON_DOWN) && Tap(s, SOTN_BUTTON_DOWN) && s->shop.cursor == 2);
    CHECK(Tap(s, SOTN_BUTTON_JUMP) && s->slots[131].state.step == 4 &&
          !s->slots[132].state.generation); // Missing optional service cannot report success.
    // Malformed profiles fail explicitly instead of reading host pointers.
    ActorPrimitivesClear(s, &s->slots[131]);
    s->slots[131].state.step = 0;
    s->slots[131].state.primitive_count = 0;
    f->lib[0x1a08] = 7;
    CHECK(!Tick(s, 0));
    SotnCharacter_Destroy(s->character);
    s->character = NULL;
    SotnActors_Destroy(s);
    return true;
}
static bool Repeat(void)
{
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s);
    s->buttons = s->tapped_buttons = SOTN_BUTTON_DOWN;
    ActorRepeatButtons(s);
    CHECK(s->repeat_buttons == SOTN_BUTTON_DOWN);
    s->tapped_buttons = 0;
    for (Uint16 i = 0; i < 16; ++i)
    {
        ActorRepeatButtons(s);
        CHECK(!s->repeat_buttons);
    }
    ActorRepeatButtons(s);
    CHECK(s->repeat_buttons == SOTN_BUTTON_DOWN);
    for (Uint16 i = 0; i < 5; ++i)
    {
        ActorRepeatButtons(s);
        CHECK(!s->repeat_buttons);
    }
    // A newly tapped right direction does not reset the already-held down timer.
    s->buttons |= SOTN_BUTTON_RIGHT;
    s->tapped_buttons = SOTN_BUTTON_RIGHT;
    ActorRepeatButtons(s);
    CHECK(s->repeat_buttons == (SOTN_BUTTON_DOWN | SOTN_BUTTON_RIGHT));
    s->buttons = s->tapped_buttons = 0;
    ActorRepeatButtons(s);
    CHECK(!s->repeat_buttons);
    SDL_free(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = Flow(f) && Repeat();
    SDL_free(f);
    if (ok)
        SDL_Log("SOTN_LIBRARY_SHOP_MENU_TEST: PASS");
    return ok ? 0 : 1;
}
