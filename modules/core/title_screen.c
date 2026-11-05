// SPDX-License-Identifier: AGPL-3.0-or-later
// st/sel/3410C.c:HandleTitleScreen and 318CC.c:SetPrimGrey, sotn-decomp
// 7bc54d8157b6bed77e6bd15e5db9461a734fe1e5. Artwork/palettes remain private inputs.
#include <sotn/combat.h>
#include <sotn/title_screen.h>
static void TitleGrey(SotnTitleSprite *p, Uint8 value)
{
    for (Uint16 i = 0; i < 4; ++i)
        p->red[i] = p->green[i] = p->blue[i] = value;
}
static void TitleInitialize(SotnTitleScreen *s)
{
    SDL_zero(s->sprites);
    for (Uint16 i = 0; i < 3; ++i)
        s->sprites[i] = (SotnTitleSprite){.x = (Sint16)(i * 128 + 112),
                                          .y = 168,
                                          .u = 128,
                                          .v = (Uint8)(i * 16 + 176),
                                          .width = i == 2 ? 32 : 128,
                                          .height = 16,
                                          .page = 24,
                                          .clut = 32,
                                          .priority = 1,
                                          .draw_mode = 8};
    for (Uint16 i = 0; i < 4; ++i)
    {
        s->sprites[i + 3] = (SotnTitleSprite){.x = (Sint16)(i * 128 + 12),
                                              .y = 214,
                                              .u = 128,
                                              .v = (Uint8)(i * 8 + 144),
                                              .width = i == 3 ? 100 : 128,
                                              .height = 8,
                                              .page = 24,
                                              .clut = 33,
                                              .priority = 1,
                                              .draw_mode = 8};
        s->sprites[i + 7] = (SotnTitleSprite){.x = (Sint16)(i * 128),
                                              .width = 128,
                                              .height = 240,
                                              .page = (Uint16)(i + 0x88),
                                              .clut = 16,
                                              .draw_mode = 8};
        s->sprites[i + 11] = (SotnTitleSprite){.x = (Sint16)(i * 128),
                                               .y = 24,
                                               .width = 128,
                                               .height = 136,
                                               .page = (Uint16)(i + 0x8c),
                                               .priority = 2,
                                               .draw_mode = 8};
    }
    s->sprites[15] = (SotnTitleSprite){.x = 257,
                                       .y = 96,
                                       .width = 255,
                                       .height = 144,
                                       .page = 24,
                                       .clut = 34,
                                       .priority = 1,
                                       .draw_mode = 8};
    s->sprites[16] = (SotnTitleSprite){.v = 144,
                                       .width = 120,
                                       .height = 64,
                                       .page = 24,
                                       .clut = 34,
                                       .priority = 1,
                                       .draw_mode = 8};
}
static void TitleLightning(SotnTitleSprite *p, Uint16 i, const SotnTitleSprite *first,
                           Uint32 *random)
{
    switch (p->phase)
    {
    case 0:
        if (i == 15)
        {
            if (!(SotnCombat_Random(random) & 127U))
            {
                p->timer = 6;
                p->page = 25;
                ++p->phase;
            }
        }
        else if (first->phase >= 2 || (first->phase && first->timer < 3))
        {
            p->timer = (Uint8)((SotnCombat_Random(random) & 3U) + 6U);
            p->page = 25;
            ++p->phase;
        }
        break;
    case 1:
        if (!--p->timer)
        {
            p->timer = 6;
            p->page = 26;
            ++p->phase;
        }
        break;
    case 2:
        if (!--p->timer)
        {
            p->timer = (Uint8)((SotnCombat_Random(random) & 7U) + 6U);
            p->page = 27;
            ++p->phase;
        }
        break;
    case 3:
        if (!--p->timer)
        {
            p->timer = 5;
            p->page = 26;
            ++p->phase;
        }
        break;
    case 4:
        if (!--p->timer)
        {
            p->timer = 5;
            p->page = 25;
            ++p->phase;
        }
        break;
    case 5:
        if (!--p->timer)
        {
            p->timer = 6;
            p->page = 24;
            ++p->phase;
        }
        break;
    case 6:
        if (!--p->timer)
        {
            if (i == 16)
                p->phase = 0;
            else
            {
                ++p->phase;
                p->timer = 6;
                if (SotnCombat_Random(random) & 3U)
                {
                    p->phase = 1;
                    p->page = 25;
                }
            }
        }
        break;
    case 7:
        if (!--p->timer)
            p->phase = 0;
        break;
    }
    if (i == 16)
        p->width = p->page == 24 ? 120 : 112;
}
bool SotnTitleScreen_Tick(SotnTitleScreen *state, SotnTitleScreenInput in, Uint32 *random,
                          SotnTitleScreenEvents *out)
{
    if (!state || !random || !out || state->completed || state->phase > 6 || state->phase == 4 ||
        state->brightness < 0 || state->brightness > 128 || (state->brightness & 7) ||
        (state->phase == 1 && state->brightness == 128) ||
        ((state->phase == 2 || state->phase == 3) && state->brightness != 128) ||
        (state->phase == 5 && state->brightness) || (state->phase == 6 && !state->brightness))
        return SDL_SetError("Title callback requires actual retail phase and brightness words");
    for (Uint16 i = 15; i < 17; ++i)
        if (state->sprites[i].phase > 7)
            return SDL_SetError("Title lightning lost its original private phase");
    SotnTitleScreen s = *state;
    SotnTitleScreenEvents e = {0};
    Uint32 rng = *random;
    switch (s.phase)
    {
    case 0:
        TitleInitialize(&s);
        s.brightness = 0;
        ++s.phase;
        e.load_graphics = in.disc;
        e.clear = true;
        break;
    case 1:
        e.title_display = true;
        s.brightness += 8;
        for (Uint16 i = 0; i < 17; ++i)
        {
            SotnTitleSprite *p = &s.sprites[i];
            p->draw_mode = i >= 15 ? 0x15 : 4;
            TitleGrey(p, (Uint8)s.brightness);
            if (i >= 7 && i <= 10)
            {
                p->red[0] = p->blue[0] = (Uint8)(s.brightness * 3 / 4);
                p->green[0] = (Uint8)(s.brightness * 7 / 8);
            }
        }
        if (s.brightness == 128)
        {
            s.retained_word = 0x800;
            ++s.phase;
        }
        break;
    case 2:
        for (Uint16 i = 0; i < 3; ++i)
            s.sprites[i].draw_mode = (in.engine_clock & 0x30U) ? 0 : 8;
        TitleLightning(&s.sprites[15], 15, &s.sprites[15], &rng);
        TitleLightning(&s.sprites[16], 16, &s.sprites[15], &rng);
        if (in.start_tapped)
        {
            e.sound = 0x63d;
            ++s.phase;
        }
        break;
    case 3:
        s.phase = 6;
        break;
    case 5:
        e.clear = e.stage_display = e.restore_cluts = e.release_rows = e.main_menu = true;
        s.completed = true;
        break;
    case 6:
        s.brightness -= 8;
        for (Uint16 i = 0; i < 17; ++i)
        {
            s.sprites[i].draw_mode = i >= 15 ? 0x15 : 4;
            TitleGrey(&s.sprites[i], (Uint8)s.brightness);
        }
        if (!s.brightness)
        {
            s.phase = 5;
            e.memory_card = true;
        }
        break;
    }
    *state = s;
    *random = rng;
    *out = e;
    return true;
}
