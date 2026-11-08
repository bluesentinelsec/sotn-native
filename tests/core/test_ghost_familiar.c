#include <SDL3/SDL.h>
#include <sotn/game_math.h>
#include <sotn/ghost_familiar.h>
#define CHECK(v)                                                                                   \
    do                                                                                             \
    {                                                                                              \
        if (!(v))                                                                                  \
        {                                                                                          \
            SDL_Log("Line%d %s: %s", __LINE__, #v, SDL_GetError());                                \
            return false;                                                                          \
        }                                                                                          \
    } while (0)
static Uint8 overlay[0x800], exe[0x23400];
static void Put16(Uint8 *p, Uint16 value)
{
    p[0] = (Uint8)value;
    p[1] = (Uint8)(value >> 8);
}
static void Put32(Uint8 *p, Uint32 value)
{
    Put16(p, (Uint16)value);
    Put16(p + 2, (Uint16)(value >> 16));
}
static void Event(Uint16 index, Sint32 servant, Sint32 room_x, Sint32 room_y, Uint32 condition,
                  Uint32 entity, Uint32 parameter)
{
    Uint8 *p = overlay + 0x5b0 + index * 48U;
    Put32(p + 8, (Uint32)servant);
    Put32(p + 12, (Uint32)room_x);
    Put32(p + 16, (Uint32)room_y);
    Put32(p + 20, room_x == -1 ? 0xffffffffU : 0);
    Put32(p + 24, room_x == -1 ? 0xffffffffU : 4);
    Put32(p + 28, condition);
    Put32(p + 32, room_x == -1 ? 0xffffffffU : 1);
    Put32(p + 36, entity);
    Put32(p + 40, parameter);
    Put32(p + 44, room_x != -1);
}
// Authored SDK tables and a synthetic tt_001 image: CLUTs, the eight-frame loop, the ability
// table (delays 60/40, spells 17/18), the question-mark offsets and one spike-room event.
static void Fixture(void)
{
    for (Uint32 i = 0; i <= 1024; ++i)
    {
        Put16(exe + 0x1dbcc + i * 2, (Uint16)SDL_lround(4096.0 * SDL_sin(i * SDL_PI_D / 2048.0)));
        Put16(exe + 0x22a10 + i * 2, (Uint16)SDL_lround(SDL_atan(i / 1024.0) * 2048.0 / SDL_PI_D));
    }
    for (Uint32 i = 0; i < 192; ++i)
        Put16(exe + 0x1e3e0 + i * 2, (Uint16)SDL_lround(SDL_sqrt((64.0 + i) / 64.0) * 4096.0));
    for (Uint16 i = 0; i < 48; ++i)
        Put16(overlay + 0x448 + i * 2U, (Uint16)(0x100 + i));
    for (Uint16 i = 0; i < 8; ++i)
    {
        Put16(overlay + 0x4a8 + i * 4U, 2);
        Put16(overlay + 0x4aa + i * 4U, (Uint16)(0x201 + i));
    }
    Put32(overlay + 0x500, 0x801704a8U);
    Put32(overlay + 0x504, 0x801704ccU);
    for (Uint16 i = 0; i < 10; ++i)
    {
        Put32(overlay + 0x508 + i * 12U, i < 5 ? 60 : 40);
        Put32(overlay + 0x50c + i * 12U, i < 7 ? 17 : 18);
        Put32(overlay + 0x510 + i * 12U, i < 2);
    }
    for (Uint16 i = 0; i < 16; ++i)
        Put16(overlay + 0x580 + i * 2U, (Uint16)(0x7c00 + i));
    const Sint16 ox[3] = {6, 3, 8}, oy[3] = {-20, -18, -15};
    for (Uint16 i = 0; i < 3; ++i)
    {
        Put16(overlay + 0x5a0 + i * 2U, (Uint16)ox[i]);
        Put16(overlay + 0x5a8 + i * 2U, (Uint16)oy[i]);
    }
    Event(0, -1, -1, -1, 0xffffffffU, 0, 0);
    Event(1, 2, 41, 50, 0x80000000U | 66U, SOTN_GHOST_DESTROY, 15);
    Event(2, -1, -1, -1, 0xffffffffU, 0, 0);
}
static SotnSwordTarget targets[SOTN_SWORD_TARGETS];
static SotnWorldState world;
static SotnGhostInput Input(Sint32 level)
{
    return (SotnGhostInput){.overlay = {overlay, sizeof(overlay)},
                            .executable = {exe, sizeof(exe)},
                            .player = {.x = 120 * 65536, .y = 150 * 65536, .priority = 0x94},
                            .targets = targets,
                            .stats = {level, (level - 1) * 100, 1},
                            .stage = 2,
                            .allocated = true,
                            .event_source = {{overlay, sizeof(overlay)}, 0x801705b0U, 3},
                            .room = {.world = &world, .room_x = 10, .room_y = 10, .servant = 2}};
}
// One frame: slots4..7 in order, collecting requests; marks land on the stage snapshot.
static bool Frame(SotnGhostRuntime *r, SotnGhostInput in, SotnGhostEvents *all)
{
    *all = (SotnGhostEvents){0};
    for (Uint16 slot = 4; slot < 8; ++slot)
    {
        in.allocated = in.allocated || slot == 4;
        SotnGhostEvents e = {0};
        CHECK(SotnGhostRuntime_UpdateSlot(r, slot, in, &e));
        if (e.mark)
        {
            all->mark = e.mark;
            targets[e.mark - 1].flags |= 0x80000;
        }
        all->spell = e.spell ? e.spell : all->spell;
        all->sound = e.sound ? e.sound : all->sound;
    }
    return true;
}
static Sint16 X(Sint32 v)
{
    return (Sint16)(v >> 16);
}
static bool Init(void)
{
    static SotnGhostRuntime r;
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    CHECK(!SotnGhostRuntime_Init(&r, in, 4, &e) && !r.initialized);
    in.stats.level = 7;
    CHECK(!SotnGhostRuntime_Init(&r, in, 0, &e) && !r.initialized);
    in = Input(5);
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) && r.initialized && e.clut_upload);
    // Beside the player (facing right: 18 behind), 32 above, two priorities below, facing back.
    const SotnGhostSlot *s = &r.slots[0];
    CHECK(s->identity == SOTN_GHOST_DEFAULT && X(s->x) == 102 && X(s->y) == 118 &&
          s->priority == 0x92 && s->facing_left && !s->flags && !s->step);
    // The 256-word copy runs past g_ServantClut (0x8000 except each sixteenth word) and the
    // 32-word overwrite past g_GhostClut into the offsets and the dummy event's live words.
    CHECK(e.clut[0] == 0x100 && e.clut[1] == 0x8101 && e.clut[16] == 0x110 &&
          e.clut[47] == 0x812f && e.clut[48] == 0x7c00 && e.clut[63] == 0x7c0f && e.clut[64] == 6 &&
          e.clut[67] == 0 && e.clut[68] == 0xffec && e.clut[71] == 0 && e.clut[72] == 0 &&
          e.clut[76] == 0xffff && e.clut[49] == 0x7c01);
    CHECK(e.clut[100] == 0x8001); // g_GhostAbilityStats[0].makeBadAttacks, past the CLUT
    in.player.facing_left = true;
    CHECK(SotnGhostRuntime_Init(&r, in, 2, &e) && X(r.slots[0].x) == 138 &&
          !r.slots[0].facing_left);
    CHECK(SotnGhostRuntime_Init(&r, in, 1, &e) && X(r.slots[0].x) == 128 && X(r.slots[0].y) == -32);
    in.room_flags = 0x20;
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) && X(r.slots[0].x) == 192 && X(r.slots[0].y) == 160);
    in.room_flags = 0x22;
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) && X(r.slots[0].x) == 64);
    in.stage = 0x25;
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) && X(r.slots[0].x) == 192);
    // Same servant: nothing changes and no CLUT is uploaded.
    r.slots[0].step = 3;
    CHECK(SotnGhostRuntime_Init(&r, in, 3, &e) && !e.clut_upload && r.slots[0].step == 3);
    // A truncated overlay fails without mutation.
    SotnGhostRuntime before = r;
    in.overlay.size = 0x500;
    CHECK(!SotnGhostRuntime_Init(&r, in, 0, &e) && !SDL_memcmp(&r, &before, sizeof(r)));
    return true;
}
static bool Follow(SotnGhostRuntime *r)
{
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    SDL_memset(targets, 0, sizeof(targets));
    CHECK(SotnGhostRuntime_Init(r, in, 0, &e));
    CHECK(!SotnGhostRuntime_UpdateSlot(r, 3, in, &e) && !SotnGhostRuntime_UpdateSlot(r, 8, in, &e));
    CHECK(Frame(r, in, &e));
    const SotnGhostSlot *s = &r->slots[0];
    CHECK(s->step == 1 && s->flags == 0x0c020000U && s->max_angle == 512 && s->glow == 127 &&
          s->glow_step == -1 && s->opacity == 127 && s->frame == 1 && s->pose_timer == 2 &&
          s->bob == 32);
    // The pulse runs between 0x20 and 0x80; the eight-frame loop repeats.
    Uint16 frames = 0;
    Sint16 low = 0x80;
    for (Uint16 t = 0; t < 200; ++t)
    {
        CHECK(Frame(r, in, &e) && s->step == 1 && !e.mark && !e.spell);
        frames |= (Uint16)(1U << s->frame);
        low = SDL_min(low, s->glow);
    }
    CHECK(frames == 0x1fe && low == 0x20 && s->glow_step);
    // It settles near the follow point: 18 behind the player, 32 above, bobbing.
    CHECK(SDL_abs(X(s->x) - 102) <= 1 && SDL_abs(X(s->y) - 118) <= 6);
    return true;
}
static bool Attack(void)
{
    static SotnGhostRuntime r;
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    CHECK(Follow(&r));
    const SotnGhostSlot *s = &r.slots[0];
    // Targets behind the ghost (it faces left: x below its own), a shielded one (hitbox8) and
    // one marked FLAG_UNK_200000 stay out; the ghost marks and approaches the plain one.
    targets[3] = (SotnSwordTarget){0x30, 8, 0, (Sint16)(X(s->x) - 20), (Sint16)X(s->y), 40};
    targets[4] = (SotnSwordTarget){0x31, 1, 0x200000, (Sint16)(X(s->x) - 10), 118, 40};
    targets[5] = (SotnSwordTarget){0x32, 1, 0, (Sint16)(X(s->x) + 20), 118, 40};
    targets[6] = (SotnSwordTarget){0x33, 1, 0, (Sint16)(X(s->x) - 30), 128, 40};
    // Facing left, it takes targets at or right of it: target5 (mark6), not the shielded one.
    CHECK(s->facing_left);
    const bool left = true;
    const Uint16 want = 6;
    CHECK(Frame(&r, in, &e) && e.mark == want && s->step == 2 && s->target == want);
    Uint16 t = 0;
    while (s->step == 2 && t++ < 60)
        CHECK(Frame(&r, in, &e));
    CHECK(s->step == 3 && !s->frame_counter);
    // Delay 60: the ring at frame 30 (glass shards, slot5, eight rows), the attack after 60.
    for (t = 1; t < 30; ++t)
        CHECK(Frame(&r, in, &e) && !r.slots[1].identity && !e.sound);
    CHECK(SotnGhostRuntime_RowRequest(&r, 5) == 0);
    SotnGhostSlot snapshot = r.slots[0];
    CHECK(SotnGhostRuntime_UpdateSlot(&r, 4, in, &e) && r.slots[1].identity == SOTN_GHOST_CLOUD &&
          r.slots[0].attack == 5 && SotnGhostRuntime_RowRequest(&r, 5) == 8);
    (void)snapshot;
    CHECK(SotnGhostRuntime_UpdateSlot(&r, 5, in, &e) && e.sound == SOTN_GHOST_SOUND &&
          r.slots[1].step == 1 && r.cloud_timer == 29 && r.cloud_angle == 50);
    SotnGhostLease leases[3] = {0};
    CHECK(SotnGhostRuntime_Leases(&r, leases) && leases[0].count == 8 && leases[0].slot == 5 &&
          !leases[1].count && r.rows[0][0].palette == 0x143 && r.rows[0][0].page == 0x1a &&
          r.rows[0][0].draw_mode == 0x135 && r.rows[0][0].red[3] == 0x80);
    // Row k sits timer pixels out at angle + k*512 around the ghost.
    CHECK(r.rows[0][0].x[1] - r.rows[0][0].x[0] == 8 && r.rows[0][0].y[2] - r.rows[0][0].y[0] == 8);
    for (t = 31; t <= 60; ++t)
        CHECK(Frame(&r, in, &e) && !e.spell && !s->hitbox_width);
    CHECK(Frame(&r, in, &e) && e.spell == 17 && s->spell == 17 && s->hitbox_width == 8 &&
          s->hitbox_height == 8 && !s->frame_counter && !r.slots[1].identity);
    CHECK(Frame(&r, in, &e) && !s->hitbox_width);
    // The target darts away: once out of reach the next ring is released at once.
    for (t = 1; t <= 30; ++t)
        CHECK(Frame(&r, in, &e));
    CHECK(r.slots[1].identity == SOTN_GHOST_CLOUD);
    targets[want - 1].x = (Sint16)(targets[want - 1].x + (left ? -60 : 60));
    CHECK(Frame(&r, in, &e) && !r.slots[1].identity && !s->frame_counter);
    // A dead target ends the attack; the ghost returns to following.
    targets[want - 1].hit_points = 0;
    CHECK(Frame(&r, in, &e) && s->step == 1);
    // A failed ring allocation retires the ring without rows.
    targets[want - 1] = (SotnSwordTarget){0x33, 1, 0x80000, (Sint16)X(s->x), (Sint16)X(s->y), 40};
    in.allocated = false;
    for (t = 0; t < 200 && !r.slots[1].identity; ++t)
    {
        CHECK(SotnGhostRuntime_UpdateSlot(&r, 4, in, &e));
        targets[want - 1].x = X(s->x);
        targets[want - 1].y = X(s->y);
    }
    CHECK(r.slots[1].identity == SOTN_GHOST_CLOUD && SotnGhostRuntime_UpdateSlot(&r, 5, in, &e) &&
          !r.slots[1].identity && !e.sound);
    // Soul steal from level 70 (spell18) and a 40-update delay.
    in = Input(75);
    in.allocated = true;
    Uint16 spell = 0;
    for (t = 0; t < 120 && !spell; ++t)
    {
        CHECK(Frame(&r, in, &e));
        spell = e.spell;
        targets[want - 1].x = X(s->x);
        targets[want - 1].y = X(s->y);
    }
    CHECK(spell == 18);
    // A ghost facing right (the player faces left) ignores targets to its right.
    static SotnGhostRuntime right;
    SotnGhostInput other = Input(25);
    other.player.facing_left = true;
    SDL_memset(targets, 0, sizeof(targets));
    CHECK(SotnGhostRuntime_Init(&right, other, 0, &e) && Frame(&right, other, &e) &&
          !right.slots[0].facing_left);
    targets[8] = (SotnSwordTarget){0x36, 1, 0, (Sint16)(X(right.slots[0].x) + 20), 118, 40};
    CHECK(Frame(&right, other, &e) && !e.mark && right.slots[0].step == 1);
    targets[8].x = (Sint16)(X(right.slots[0].x) - 20);
    CHECK(Frame(&right, other, &e) && e.mark == 9 && right.slots[0].step == 2);
    targets[want - 1] = (SotnSwordTarget){0x33, 1, 0x80000, (Sint16)X(s->x), (Sint16)X(s->y), 40};
    // A cutscene drops the target search back to following.
    in.cutscene = true;
    CHECK(Frame(&r, in, &e) && s->step == 1 && !e.mark);
    return true;
}
static bool Confusion(void)
{
    static SotnGhostRuntime r;
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    CHECK(Follow(&r));
    const SotnGhostSlot *s = &r.slots[0];
    in.player.status = 1; // bat form
    CHECK(Frame(&r, in, &e) && s->step == 4 && !s->confused_step);
    // Hover 1+61, turn four times (16 each), hover 61, then the question marks.
    Uint16 turns = 0, t = 0;
    bool facing = s->facing_left;
    while (!r.slots[1].identity && t++ < 400)
    {
        CHECK(Frame(&r, in, &e));
        turns = (Uint16)(turns + (s->facing_left != facing));
        facing = s->facing_left;
    }
    CHECK(turns == 4 && r.slots[1].identity == SOTN_GHOST_CONFUSED && s->confused == 5 &&
          r.slots[1].step == 1 && r.rows[0][0].draw_mode == 0x102 &&
          r.rows[0][1].draw_mode == 0x10a && r.rows[0][2].draw_mode == 0x10a);
    CHECK(r.rows[0][0].y[0] == X(s->y) - 20 && r.rows[0][1].y[0] == X(s->y) - 18);
    for (t = 0; t < 11; ++t)
        CHECK(Frame(&r, in, &e));
    CHECK(r.rows[0][1].draw_mode == 0x102 && r.rows[0][2].draw_mode == 0x10a);
    // After sixty more the ghost rises off the top and parks at (128, -16).
    for (t = 0; t < 200 && s->step != 6; ++t)
        CHECK(Frame(&r, in, &e));
    CHECK(s->step == 6 && X(s->x) == 128 && X(s->y) == -16 && !r.slots[1].identity);
    // Leaving bat form brings it back; Axe Armor confuses it as well.
    in.player.status = 0;
    CHECK(Frame(&r, in, &e) && s->step == 1);
    in.player.status = 0x1000000;
    CHECK(Frame(&r, in, &e) && s->step == 4);
    for (t = 0; t < 400 && !r.slots[1].identity; ++t)
        CHECK(Frame(&r, in, &e));
    CHECK(r.slots[1].identity == SOTN_GHOST_CONFUSED);
    in.player.status = 0;
    CHECK(SotnGhostRuntime_UpdateSlot(&r, 4, in, &e) && s->step == 1 && !r.slots[1].identity);
    return true;
}
static bool RoomEvent(void)
{
    static SotnGhostRuntime r;
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    CHECK(Follow(&r));
    // The lit spike room's flag suppresses the event; the unlit one raises the ghost's layer.
    in.room.room_x = 41;
    in.room.room_y = 50;
    in.room.camera_y = 4;
    world.flags[66] = 1;
    CHECK(Frame(&r, in, &e) && Frame(&r, in, &e) && !r.destroyed);
    world.flags[66] = 0;
    in.room.room_x = 10;
    CHECK(Frame(&r, in, &e));
    in.room.room_x = 41;
    Uint16 t = 0;
    while (!r.destroyed && t++ < 10)
        CHECK(Frame(&r, in, &e));
    CHECK(r.destroyed && !r.slots[1].identity);
    in.player.priority = 0x40;
    CHECK(Frame(&r, in, &e) && r.slots[0].priority == 0x3e);
    return true;
}
static bool Lifecycle(void)
{
    static SotnGhostRuntime r;
    SotnGhostEvents e = {0};
    SotnGhostInput in = Input(5);
    CHECK(!SotnGhostRuntime_UpdateSlot(&r, 4, in, &e));
    CHECK(SotnGhostRuntime_Translate(&r, 1, 1, 0, 0, false));
    // A room entry before the first update retires the ghost (no FLAG_UNK_20000 yet).
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) &&
          SotnGhostRuntime_Translate(&r, 0, 0, 0, 0, true) && !r.slots[0].identity);
    CHECK(SotnGhostRuntime_Init(&r, in, 0, &e) && Frame(&r, in, &e));
    const Sint32 x = r.slots[0].x;
    CHECK(SotnGhostRuntime_Translate(&r, 3, -2, 0, 0, false) && r.slots[0].x == x + 3 * 65536);
    CHECK(SotnGhostRuntime_Translate(&r, 0, 0, 5 * 65536, 0, true) &&
          r.slots[0].x == x - 2 * 65536);
    // Unknown identities and a body outside slot4 fail without mutation.
    r.slots[2].identity = 0xd5;
    SotnGhostRuntime before = r;
    CHECK(!SotnGhostRuntime_UpdateSlot(&r, 6, in, &e) && !SDL_memcmp(&r, &before, sizeof(r)));
    r.slots[2].identity = SOTN_GHOST_DEFAULT;
    CHECK(!SotnGhostRuntime_UpdateSlot(&r, 6, in, &e));
    r.slots[2].identity = 0;
    in.stats.experience = 50000;
    CHECK(!SotnGhostRuntime_UpdateSlot(&r, 4, in, &e));
    CHECK(SotnGhostRuntime_Clear(&r) && !r.slots[0].identity && r.initialized);
    return true;
}
int main(void)
{
    Fixture();
    if (!Init() || !Attack() || !Confusion() || !RoomEvent() || !Lifecycle())
        return 1;
    SDL_Log("SOTN_GHOST_FAMILIAR_TEST: PASS");
    return 0;
}
