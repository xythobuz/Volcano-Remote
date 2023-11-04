/*
 * text.c
 *
 * Copyright (c) 2023 Thomas Buck (thomas@xythobuz.de)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * See <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include "log.h"
#include "lcd.h"
#include "text.h"

typedef struct {
    struct text_conf *options;
    uint16_t anchor;
} state_t;

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha,
                           void *state) {
    state_t *s = (state_t*)state;

    if (y < 0 || y >= s->options->height) return;
    if (x < 0 || x + count >= s->options->width) return;

    while (count--) {
        uint32_t c = RGB_565(alpha, alpha, alpha);
        lcd_write_point(240 - y - 1, x, c);
        x++;
    }
}

static uint8_t character_callback(int16_t x, int16_t y, mf_char character,
                                  void *state) {
    state_t *s = (state_t*)state;
    uint8_t w = mf_render_character(s->options->font->font, x, y, character, pixel_callback, state);
    return w;
}

static bool line_callback(const char *line, uint16_t count, void *state) {
    state_t *s = (state_t*)state;

    if (s->options->bg != TEXT_BG_NONE) {
        int16_t width = mf_get_string_width(s->options->font->font, line, count, false) + 2 * s->options->margin;
        int16_t line_height = s->options->font->font->line_height;

        if (s->options->alignment == MF_ALIGN_LEFT) {
            lcd_write_rect(240 - s->options->y - 1 - line_height,
                           s->options->x,
                           240 - s->options->y - 1,
                           s->options->x + width,
                           s->options->bg);
        } else if (s->options->alignment == MF_ALIGN_CENTER) {
            lcd_write_rect(240 - s->options->y - 1 - line_height,
                           s->options->x + s->options->width / 2 - width / 2,
                           240 - s->options->y - 1,
                           s->options->x + s->options->width / 2 + width / 2,
                           s->options->bg);
        } else if (s->options->alignment == MF_ALIGN_RIGHT) {
            lcd_write_rect(240 - s->options->y - 1 - line_height,
                           s->options->x + s->options->width - width,
                           240 - s->options->y - 1,
                           s->options->x + s->options->width,
                           s->options->bg);
        }
    }

    if (s->options->justify) {
        mf_render_justified(s->options->font->font, s->anchor + s->options->x, s->options->y,
                            s->options->width - s->options->margin * 2,
                            line, count, character_callback, state);
    } else {
        mf_render_aligned(s->options->font->font, s->anchor + s->options->x, s->options->y,
                          s->options->alignment, line, count,
                          character_callback, state);
    }

    s->options->y += s->options->font->font->line_height;
    return true;
}

void text_prepare_font(struct text_font *tf) {
    if (!tf) {
        debug("invalid param");
        return;
    }

    const struct mf_font_s *font = mf_find_font(tf->fontname);
    if (!font) {
        debug("No such font: %s", tf->fontname);
        return;
    }

    tf->font = font;

    // TODO
    //struct mf_scaledfont_s scaledfont;
    //if (tf->scale > 1) {
    //    mf_scale_font(&scaledfont, font, tf->scale, tf->scale);
    //    tf->font = scaledfont.font;
    //}
}

void text_draw(struct text_conf *tc) {
    if ((!tc) || (!tc->font) || (!tc->font->font)) {
        debug("invalid param");
        return;
    }

    debug("'%s' %d", tc->text, tc->y);

    state_t state;
    state.options = tc;

    if (tc->alignment == MF_ALIGN_LEFT) {
        state.anchor = tc->margin;
    } else if (tc->alignment == MF_ALIGN_CENTER) {
        state.anchor = tc->width / 2;
    } else if (tc->alignment == MF_ALIGN_RIGHT) {
        state.anchor = tc->width - tc->margin;
    }

    mf_wordwrap(tc->font->font, tc->width - 2 * tc->margin,
                tc->text, line_callback, &state);
}
