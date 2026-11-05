#include "actors_internal.h"
#include "character_fixture.h"
#include <sotn/food_popup.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("line%d %s %s", __LINE__, #v, SDL_GetError());                                 \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static void Put32(Uint8 *p, Uint32 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(value >> (i * 8));
}
static bool Exercise(void)
{
    Uint8 dra[4096] = {0};
    Put32(dra + 0xc0, 0x800a0100);
    Put32(dra + 0x100 + 29 * 52, 0x800a0f00);
    const Uint8 plain[] = {'A', 'B', 255, 0}, accent[] = {'A', 255, 240, 'B', 255, 0};
    SDL_memcpy(dra + 0xf00, plain, sizeof(plain));
    SotnFoodPopup popup = {0};
    CHECK(SotnFoodPopup_Build((SotnDataView){dra, sizeof(dra)}, 29, &popup) && popup.count == 6 &&
          popup.rows[0].width == 52 && popup.rows[0].quad && popup.rows[0].vertex_blue[0] == 175 &&
          popup.rows[0].vertex_blue[2] == 0 && popup.rows[4].x == 23 * 65536 &&
          popup.rows[4].y == 212 * 65536 && popup.rows[5].x == 31 * 65536 &&
          popup.rows[5].y == 212 * 65536);
    SDL_memcpy(dra + 0xf00, accent, sizeof(accent));
    CHECK(SotnFoodPopup_Build((SotnDataView){dra, sizeof(dra)}, 29, &popup) && popup.count == 7 &&
          popup.rows[5].x == 23 * 65536 && popup.rows[5].y == 204 * 65536 &&
          popup.rows[6].x == 35 * 65536);
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    CHECK(s);
    s->dra = (SotnDataView){dra, sizeof(dra)};
    s->entered = true;
    CHECK(SotnActors_ShowFoodText(s, 29) && SotnActors_FoodTextRows(s) == 7 &&
          s->food_text.timer == 304);
    SotnPrimitiveSlots held = {0};
    CHECK(SotnActors_PrimitiveSlots(s, &held));
    const Uint32 generation = s->food_text.generation;
    for (Uint16 i = 0; i < 7; ++i)
    {
        SotnActorPrimitive p = {0};
        CHECK(SotnActors_GetFoodText(s, i, &p) && p.active && p.screen_space && p.quad &&
              p.priority_set);
    }
    CHECK(s->food_text.timer == 304 && s->food_text.generation == generation);
    for (Uint16 i = 0; i < 303; ++i)
        CHECK(ActorFoodTextTick(s) && SotnActors_FoodTextRows(s) == 7);
    CHECK(s->food_text.timer == 1 && ActorFoodTextTick(s) && !SotnActors_FoodTextRows(s));
    SotnPrimitiveSlots empty = {0};
    CHECK(SotnActors_PrimitiveSlots(s, &empty));
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_WORDS; ++i)
        CHECK(!empty.occupied[i]);
    // A pickup replaces its text before later player children allocate their rows.
    SDL_memset(dra + 0xf00, 'A', 20);
    dra[0xf14] = 255;
    dra[0xf15] = 0;
    CHECK(SotnActors_ShowFoodText(s, 29) && s->food_text.count == 24);
    SotnBatRuntime *runtime = SDL_calloc(1, sizeof(*runtime));
    CHECK(runtime && SotnActors_GetFoodTextPlan(s, &runtime->food_text) &&
          SotnActors_PrimitiveSlots(s, &empty));
    SDL_memcpy(dra + 0xf00, plain, sizeof(plain));
    CHECK(SotnFoodTextPlan_Replace(&runtime->food_text, s->dra, 29, &empty));
    SotnBatRuntimeChild *blink = &runtime->children[24];
    *blink = (SotnBatRuntimeChild){
        .kind = 11, .generation = 123, .initialized = true, .primitive_count = 8};
    bool allocated = false;
    CHECK(SotnPrimitiveSlots_Reserve(&empty, 8, blink->primitives, &allocated) && allocated &&
          blink->primitives[0] == 6 && SotnActors_CommitBatPrimitives(s, runtime) &&
          s->food_text.count == 6 && s->primitives[6].ability_kind == 11 &&
          s->primitives[13].owner == 123 && !s->primitives[14].owner);
    CHECK(SotnActors_GetFoodTextPlan(s, &runtime->food_text) &&
          SotnActors_PrimitiveSlots(s, &empty));
    SDL_memset(dra + 0xf00, 'A', 20);
    dra[0xf14] = 255;
    dra[0xf15] = 0;
    CHECK(SotnFoodTextPlan_Replace(&runtime->food_text, s->dra, 29, &empty));
    SotnActors *before = SDL_malloc(sizeof(*before));
    CHECK(before);
    SDL_memcpy(before, s, sizeof(*before));
    runtime->target_marks[0] = 1;
    CHECK(!SotnActors_CommitBatPrimitives(s, runtime) && !SDL_memcmp(before, s, sizeof(*before)));
    SDL_free(before);
    SDL_free(runtime);
    CHECK(ActorFoodTextClear(s));
    SDL_memset(s->primitives, 0, sizeof(s->primitives));
    SDL_memcpy(dra + 0xf00, plain, sizeof(plain));
    CHECK(SotnActors_ShowFoodText(s, 29) && s->food_text.count == 6);
    for (Uint16 i = 0; i < SOTN_PRIMITIVE_SLOTS; ++i)
        if (!s->primitives[i].owner)
            s->primitives[i].owner = 0xdeadbeef;
    const Uint16 first = s->food_text.rows[0];
    SDL_memset(dra + 0xf00, 'A', 20);
    dra[0xf14] = 255;
    dra[0xf15] = 0;
    CHECK(SotnActors_ShowFoodText(s, 29) && !SotnActors_FoodTextRows(s) && !s->food_text.timer &&
          !s->primitives[first].owner && s->primitives[1000].owner == 0xdeadbeef);
    SDL_memset(s->primitives, 0, sizeof(s->primitives));
    CHECK(SotnActors_ShowFoodText(s, 29));
    const Uint16 row = s->food_text.rows[0];
    s->primitives[row].owner++;
    SotnActorPrimitive untouched = {.width = 123}, expected = untouched;
    CHECK(!SotnActors_GetFoodText(s, 0, &untouched) &&
          !SDL_memcmp(&untouched, &expected, sizeof(expected)) && !ActorFoodTextClear(s) &&
          s->food_text.count == 24);
    s->primitives[row].owner--;
    CHECK(ActorFoodTextClear(s));
    Put32(dra + 0x100 + 29 * 52, 0x800a0fff);
    dra[0xfff] = 255;
    popup.count = 123;
    CHECK(!SotnFoodPopup_Build((SotnDataView){dra, sizeof(dra)}, 29, &popup) && popup.count == 123);
    SDL_free(s);
    return true;
}
static bool SharedInventory(void)
{
    Uint8 *dra = SDL_calloc(1, 0x11000);
    SotnActors *s = SDL_calloc(1, sizeof(*s));
    SotnBatRuntime *runtime = SDL_calloc(1, sizeof(*runtime));
    SotnBatRuntime *before = SDL_malloc(sizeof(*before));
    SotnCharacter *character = NULL;
    CHECK(dra && s && runtime && before);
    TestCharacter_Fill(dra);
    s->dra = (SotnDataView){dra, 0x11000};
    s->entered = true;
    CHECK(SotnCharacter_Create(s->dra, &character));
    for (Uint16 i = 1; i <= 2; ++i)
    {
        const Uint16 parameter = (Uint16)((30U + i) << 8);
        runtime->children[i] = (SotnBatRuntimeChild){.kind = i == 1 ? 0xe1 : 0xf1,
                                                     .parameter = parameter,
                                                     .generation = i,
                                                     .initialized = true,
                                                     .food = true,
                                                     .state.food = {.x = 40 * 65536,
                                                                    .y = 80 * 65536,
                                                                    .timer = 2,
                                                                    .parameter = parameter,
                                                                    .priority = 150,
                                                                    .phase = 2,
                                                                    .inventory = true,
                                                                    .flags = 0x08000000}};
    }
    // A later valid cold Blink reaches its callback, then rejects an incomplete image.
    runtime->children[24] = (SotnBatRuntimeChild){
        .kind = 11, .parameter = 0x4400, .generation = 3, .state.blink.parameter = 0x44};
    SotnFoodTextPlan food = {0};
    SotnPrimitiveSlots slots = {0};
    CHECK(SotnActors_GetFoodTextPlan(s, &food) && SotnActors_PrimitiveSlots(s, &slots));
    SotnBatRuntimeInput input = {.dra = s->dra,
                                 .food_text = &food,
                                 .primitive_slots = slots,
                                 .image.width = 1,
                                 .player = {.x = 40 * 65536,
                                            .y = 80 * 65536,
                                            .priority = 148,
                                            .hp = 70,
                                            .animation = {.hitbox_width = 8, .hitbox_height = 16}}};
    SotnCharacterState prior = {0}, after = {0};
    CHECK(SotnCharacter_Get(character, &prior, NULL));
    *before = *runtime;
    CHECK(!SotnBatRuntime_Tick(runtime, character, input));
    CHECK(SDL_strstr(SDL_GetError(), "geometry"));
    CHECK(!SDL_memcmp(before, runtime, sizeof(*before)));
    CHECK(SotnCharacter_Get(character, &after, NULL));
    CHECK(!SDL_memcmp(&prior, &after, sizeof(prior)));
    CHECK(!s->food_text.count && !s->effect_serial);
    runtime->children[24] = (SotnBatRuntimeChild){0};
    CHECK(SotnBatRuntime_Tick(runtime, character, input) &&
          SotnCharacter_Get(character, &after, NULL) && after.hand_count[31] == 1 &&
          after.hand_count[32] == 1 && runtime->sound_count == 2 && runtime->sounds[0] == 0x67c &&
          runtime->sounds[1] == 0x67c && runtime->food_text.lease.generation == 2 &&
          !s->food_text.count && SotnActors_CommitBatPrimitives(s, runtime) &&
          s->food_text.generation == 2 && s->food_text.count == 8 && s->food_text.timer == 304);
    CHECK(SotnActors_GetFoodTextPlan(s, &food) &&
          SotnActors_PrimitiveSlots(s, &input.primitive_slots));
    input.player.step = SOTN_PLAYER_SOUL;
    runtime->children[31] =
        (SotnBatRuntimeChild){.kind = 53,
                              .generation = 3,
                              .initialized = true,
                              .group = 7,
                              .state.attack = {.hit_type = 0x82, .hit_effect = 2, .active = true}};
    CHECK(SotnBatRuntime_Tick(runtime, character, input) && runtime->soul_attack &&
          runtime->player_hitbox_state == 1);
    runtime->children[1] = (SotnBatRuntimeChild){.kind = 0xe0,
                                                 .parameter = 7 << 8,
                                                 .generation = 4,
                                                 .initialized = true,
                                                 .food = true,
                                                 .state.food = {.x = 40 * 65536,
                                                                .y = 80 * 65536,
                                                                .parameter = 7 << 8,
                                                                .item = 36,
                                                                .priority = 150,
                                                                .phase = 4,
                                                                .timer = 1,
                                                                .flags = 0x08000000}};
    CHECK(SotnBatRuntime_Tick(runtime, character, input) && runtime->soul_attack &&
          !runtime->player_hitbox_state && runtime->children[31].state.attack.hit_type == 0x82 &&
          runtime->food_event_count == 1 && runtime->food_events[0].poison);
    CHECK(ActorFoodTextClear(s));
    SotnCharacter_Destroy(character);
    SDL_free(dra);
    SDL_free(s);
    SDL_free(runtime);
    SDL_free(before);
    return true;
}
int main(void)
{
    return Exercise() && SharedInventory() ? 0 : 1;
}
