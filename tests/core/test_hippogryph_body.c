#include "actors_internal.h"
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
    Uint8 dra[0x1000], stage[0x2000], tiles[32 * 16 * 2], types[2];
} Fixture;
static void Put16(Uint8 *p, Uint16 v)
{
    p[0] = (Uint8)v;
    p[1] = (Uint8)(v >> 8);
}
static void Put32(Uint8 *p, Uint32 v)
{
    for (Uint16 i = 0; i < 4; ++i)
        p[i] = (Uint8)(v >> (i * 8U));
}
static void Animation(Fixture *f, size_t at, Uint8 frame, bool loop)
{
    f->stage[at] = 1;
    f->stage[at + 1] = frame;
    f->stage[at + 2] = loop ? 0 : 255;
}
static Actor Body(Uint16 step, bool facing)
{
    return (Actor){.state = {.id = 0x22,
                             .generation = 1,
                             .step = step,
                             .x = 128 * 65536,
                             .y = 183 * 65536,
                             .facing_left = facing,
                             .priority = 149,
                             .animation_set = 0x8003,
                             .palette = 0x200,
                             .texture_offset = 72,
                             .hit_points = 100,
                             .hitbox_state = 3},
                   .slot = 20};
}
static bool Open(Fixture *f, SotnActors **out)
{
    SDL_zero(*f);
    Put32(f->stage + 28, 0x80180040);
    for (Uint16 i = 0; i < 106; ++i)
        Put32(f->stage + 0x40 + i * 4U, 0x80180300);
    Put16(f->stage + 0x300, 0xfffe);
    Put16(f->stage + 0x302, 0xfffe);
    Put16(f->stage + 0x30a, 0xffff);
    Put16(f->stage + 0x30c, 0xffff);
    Put32(f->dra + 0x98, 0x800a0200);
    Put16(f->dra + 0x204, 100);
    Put16(f->dra + 0x20c, 3);
    Put16(f->stage + 0x4d8, 0x8003);
    Put16(f->stage + 0x4de, 0x200);
    Animation(f, 0x804, 3, false);
    Animation(f, 0x824, 1, true);
    Animation(f, 0x830, 6, true);
    Animation(f, 0x844, 6, false);
    Animation(f, 0x858, 30, false);
    Animation(f, 0x884, 94, true);
    Animation(f, 0x88c, 20, true);
    Animation(f, 0x894, 21, false);
    Animation(f, 0x89c, 22, false);
    Animation(f, 0x8ac, 23, true);
    Animation(f, 0x8bc, 41, false);
    Animation(f, 0x8d4, 49, false);
    Animation(f, 0x8fc, 33, false);
    Animation(f, 0x91c, 80, true);
    Animation(f, 0x928, 80, false);
    Animation(f, 0x938, 42, false);
    Animation(f, 0x944, 44, false);
    Animation(f, 0x950, 80, false);
    Animation(f, 0x96c, 94, false);
    Animation(f, 0x98c, 77, false);
    Animation(f, 0xa2c, 93, true);
    Animation(f, 0xa48, 96, true);
    Put16(f->stage + 0xaae, 9);
    Put16(f->stage + 0xadc, 0);
    Put16(f->stage + 0xade, 16);
    Put16(f->stage + 0xae0, 255);
    Put16(f->stage + 0xae4, 0);
    Put16(f->stage + 0xae6, 16);
    Put16(f->stage + 0xae8, 255);
    for (Uint16 i = 0; i < 16; ++i)
        f->stage[0xaec + i] = (Uint8)(i & 1U ? 12 : 20);
    f->types[1] = 3;
    for (Uint32 i = 32 * 12; i < 32 * 16; ++i)
        f->tiles[i * 2] = 1;
    CHECK(SotnActors_CreateStage((SotnDataView){f->dra, sizeof(f->dra)},
                                 (SotnDataView){f->stage, sizeof(f->stage)}, 25, out));
    (*out)->map =
        (SotnCollisionMap){{f->tiles, sizeof(f->tiles)}, {f->types, sizeof(f->types)}, 32, 16};
    SotnActorLayout *layout = NULL;
    CHECK(SotnActorLayout_OpenStage((*out)->nz0, 25, 0, &layout));
    const bool entered = SotnActors_Enter(*out, layout, (*out)->map, 0, 0);
    SotnActorLayout_Close(layout);
    CHECK(entered);
    (*out)->player =
        (SotnPlayerState){.hp = 70, .x = 300 * 65536, .y = 135 * 65536, .priority = 148};
    return true;
}
static bool EntryAndTakeoff(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Body(0, false);
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 1 && a.state.priority == 149 &&
          a.hippogryph.saved_priority == 149 && a.hippogryph.phase == 1 && a.state.facing_left);
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 1 && !a.state.hitbox_state);
    s->player.x = 160 * 65536;
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 3 && a.state.hitbox_state == 3 &&
          s->hippogryph_encounter == 1 &&
          (s->world->time_attack_visited & (1U << SOTN_TIME_HIPPOGRYPH)));
    CHECK(s->slots[21].state.id == 0x23 && s->slots[21].enemy_state.owner_generation == 1);
    bool takeoff = false;
    for (Uint16 i = 0; i < 200 && a.state.step != 6; ++i)
    {
        CHECK(ActorHippogryphBody(s, &a));
        takeoff |= a.state.step == 4;
    }
    CHECK(takeoff && a.state.step == 6 && s->slots[160].state.id == 0x22 &&
          (s->slots[160].state.step == 48 || s->slots[160].state.step == 50) &&
          s->slots[160].state.flags == 0xc8202000U && !s->slots[160].state.facing_left);
    SotnActors_Destroy(s);
    return true;
}
static bool FlightAndLanding(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Body(6, true);
    a.state.x = 0;
    a.state.y = 60 * 65536;
    s->player.x = 400 * 65536;
    for (Uint16 i = 0; i < 80; ++i)
        CHECK(ActorHippogryphBody(s, &a));
    CHECK(a.state.step == 22 && !a.vx && !a.vy && !a.hippogryph.timer);
    a = Body(8, false);
    bool contact = false;
    for (Uint16 i = 0; i < 20 && a.state.step != 10; ++i)
    {
        CHECK(ActorHippogryphBody(s, &a));
        contact |= a.hippogryph.phase != 0;
    }
    CHECK(contact && a.state.step == 10 && Pixel(a.state.y) == 183);
    CHECK(ActorHippogryphBody(s, &a) && (a.state.step == 12 || a.state.step == 20));
    a = Body(16, true);
    for (Uint16 i = 0; i < 100 && a.state.step != 6; ++i)
        CHECK(ActorHippogryphBody(s, &a));
    CHECK(a.state.step == 6 && a.hippogryph.timer == 64);
    a = Body(13, true);
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 13 && a.vx == 81920 + 7168 &&
          a.hippogryph.timer == 127);
    a.hit_params = 1;
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 32 && a.vx == -98304 && a.vy == -245760);
    SotnActors_Destroy(s);
    return true;
}
static bool BreathAndObjects(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Body(12, false);
    CHECK(ActorHippogryphBody(s, &a) && ActorHippogryphBody(s, &a) && a.substep == 1 &&
          a.hippogryph.timer == 64 && s->combat.sounds[0] == 0x6c3);
    for (Uint16 i = 0; i < 64; ++i)
        CHECK(ActorHippogryphBody(s, &a));
    CHECK(a.state.step == 10 && !a.hippogryph.phase);
    Uint16 lower = 0, upper = 0;
    for (Uint16 i = 96; i < 128; ++i)
        lower += (Uint16)(s->slots[i].state.generation != 0);
    for (Uint16 i = 160; i < 192; ++i)
        upper += (Uint16)(s->slots[i].state.generation != 0);
    CHECK(lower == 16 && upper == 32 && s->slots[160].state.parameters == 0 &&
          s->slots[160].state.x == 99 * 65536 && s->slots[160].state.y == 181 * 65536);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    a = Body(20, true);
    CHECK(ActorHippogryphBody(s, &a) && a.hippogryph.phase == 1 && s->slots[96].state.id == 0x25 &&
          s->slots[97].state.id == 0x25 && !s->slots[98].state.generation &&
          !s->slots[96].state.facing_left && s->slots[96].state.x == 112 * 65536 &&
          s->slots[96].vx == 32768 && s->slots[97].vx == 0 &&
          s->slots[96].hippogryph.timer >= 192 && s->slots[97].hippogryph.timer >= 192);
    SotnActors_Destroy(s);
    return true;
}
static bool GrabAndDrop(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Body(18, false);
    a.substep = 1;
    a.state.x = 200 * 65536 + 5;
    a.state.y = 100 * 65536 + 7;
    s->player.x = 205 * 65536 + 11;
    s->player.y = 120 * 65536 + 13;
    CHECK(ActorHippogryphBody(s, &a) && s->motion.grab_changed &&
          s->motion.grab.state == SOTN_GRAB_START && s->motion.grab.damage == 16 &&
          s->motion.align_horizontal && s->motion.align_vertical &&
          s->player.x == 205 * 65536 + 11 && s->player.y == 120 * 65536 + 13 &&
          a.state.x == 205 * 65536 + 5 && a.state.y == 88 * 65536 + 7);
    Uint16 pulses = 0;
    bool climb = false;
    for (Uint16 i = 0; i < 150 && a.state.step != 6; ++i)
    {
        if (s->player.grab.state != SOTN_GRAB_NONE)
            s->player.grab.state = SOTN_GRAB_HELD;
        s->motion = (SotnActorMotion){0};
        CHECK(ActorHippogryphBody(s, &a));
        climb |= a.substep == 3;
        pulses += (Uint16)(s->motion.grab_changed && s->motion.grab.state == SOTN_GRAB_DAMAGE);
    }
    CHECK(climb && pulses > 0 && a.state.step == 6 && s->motion.grab_changed &&
          s->player.grab.state == SOTN_GRAB_NONE && s->motion.grab.state == SOTN_GRAB_NONE);
    a = Body(18, false);
    a.substep = 3;
    a.state.y = 70 * 65536;
    s->player.grab = (SotnPlayerGrab){0};
    s->motion = (SotnActorMotion){0};
    CHECK(ActorHippogryphBody(s, &a) && a.state.step == 6 && s->motion.grab_changed &&
          s->motion.grab.state == SOTN_GRAB_NONE && !s->motion.grab.damage &&
          !s->player.grab.damage);
    SotnActors_Destroy(s);
    return true;
}
static bool Failures(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor a = Body(6, true);
    CHECK(!ActorHippogryphBody(NULL, &a) && !ActorHippogryphBody(s, NULL));
    a.state.flags = 0x100;
    CHECK(!ActorHippogryphBody(s, &a));
    a = Body(6, true);
    a.substep = 1;
    a.state.x = SDL_MAX_SINT32;
    a.vx = 1;
    CHECK(!ActorHippogryphBody(s, &a));
    a = Body(8, true);
    a.substep = 2;
    for (size_t i = 0xadc; i < 0xb10; ++i)
        f->stage[i] = 0;
    CHECK(!ActorHippogryphBody(s, &a));
    s->stage_id = 6;
    CHECK(!ActorHippogryphBody(s, &a));
    SotnActors_Destroy(s);
    return true;
}
static bool LinkedDamage(Fixture *f)
{
    SotnActors *s = NULL;
    CHECK(Open(f, &s));
    Actor *body = &s->slots[20], *weapon = NULL;
    *body = Body(13, false);
    body->state.x = 100 * 65536;
    body->state.y = 135 * 65536;
    body->state.frame = 5;
    body->state.hitbox_width = 20;
    body->state.hitbox_height = 8;
    body->dynamic = true;
    body->enemy.defense = 2;
    CHECK(ActorOwnedSpawnAt(s, body, 0x23, 0, 21, &weapon));
    // Synthetic selector chooses the tall upper hurtbox, outside the main body's box.
    f->stage[0xb11] = 5;
    CHECK(ActorHippogryphAttack(s, weapon));
    SotnPlayerState player = {.hp = 70,
                              .priority = 148,
                              .x = 116 * 65536,
                              .y = 83 * 65536,
                              .attack = {.active = true,
                                         .width = 1,
                                         .height = 1,
                                         .power = 16,
                                         .hit_type = 2,
                                         .hit_effect = 1,
                                         .invincibility_frames = 8,
                                         .stun_frames = 6}};
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(body->state.hit_points < 100 && body->state.hit_points > 0 &&
          weapon->state.hit_points == 0x7ffe && body->state.step == 32 && !body->stun &&
          !weapon->stun && body->cooldown[0] == 8 && weapon->cooldown[0] == 9);
    const Sint16 hp = body->state.hit_points;
    CHECK(SotnActors_Tick(s, player, 0, 0) && body->state.hit_points == hp);
    ++body->state.generation;
    CHECK(SotnActors_Tick(s, player, 0, 0) && body->state.hit_points == hp &&
          !s->slots[21].state.generation);
    SotnActors_Destroy(s);
    CHECK(Open(f, &s));
    body = &s->slots[20];
    *body = Body(2, false);
    body->state.x = 100 * 65536;
    body->state.y = 135 * 65536;
    body->state.frame = 5;
    body->state.hitbox_width = 20;
    body->state.hitbox_height = 8;
    body->dynamic = true;
    CHECK(ActorOwnedSpawnAt(s, body, 0x23, 0, 21, &weapon));
    f->stage[0xb11] = 5;
    CHECK(ActorHippogryphAttack(s, weapon));
    player.x = 100 * 65536;
    player.y = 135 * 65536;
    CHECK(SotnActors_Tick(s, player, 0, 0));
    CHECK(body->state.hit_points < 100 && weapon->state.hit_points == 0x7ffe && body->stun == 5 &&
          weapon->stun == 5 && body->cooldown[0] == 8 && weapon->cooldown[0] == 8);
    SotnActors_Destroy(s);
    return true;
}
int main(void)
{
    Fixture *f = SDL_calloc(1, sizeof(*f));
    if (!f)
        return 1;
    const bool ok = EntryAndTakeoff(f) && FlightAndLanding(f) && BreathAndObjects(f) &&
                    GrabAndDrop(f) && LinkedDamage(f) && Failures(f);
    SDL_free(f);
    if (!ok)
        return 1;
    SDL_Log("SOTN_HIPPOGRYPH_BODY: PASS");
    return 0;
}
