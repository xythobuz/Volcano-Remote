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
    int y;
} state_t;

static uint32_t blend(uint32_t fg_c, uint32_t bg_c, uint8_t alpha) {
    float bg[4] = { RGB_565_REV(bg_c), 1.0f };
    float fg[4] = { RGB_565_REV(fg_c), alpha / 255.0f };
    float r[4];
    r[3] = 1.0f - (1.0f - fg[3]) * (1.0f - bg[3]);
    if (r[3] < 1.0e-6f) {
        r[0] = 0.0f;
        r[1] = 0.0f;
        r[2] = 0.0f;
    } else {
        r[0] = fg[0] * fg[3] / r[3] + bg[0] * bg[3] * (1.0f - fg[3]) / r[3];
        r[1] = fg[1] * fg[3] / r[3] + bg[1] * bg[3] * (1.0f - fg[3]) / r[3];
        r[2] = fg[2] * fg[3] / r[3] + bg[2] * bg[3] * (1.0f - fg[3]) / r[3];
    }

    return RGB_565((uint32_t)(r[0] * 255.0f),
                   (uint32_t)(r[1] * 255.0f),
                   (uint32_t)(r[2] * 255.0f));
}

static void pixel_callback(int16_t x, int16_t y, uint8_t count, uint8_t alpha,
                           void *state) {
    state_t *s = (state_t*)state;

    if ((y < 0) || (y >= (s->options->y + s->options->height)
        || (y < s->options->y)
        || (y >= LCD_HEIGHT)) || (x >= LCD_WIDTH)
        || (x < 0) || ((x + count) >= (s->options->x + s->options->width))) {
        return;
    }

    while (count--) {
        lcd_write_point(x, y,
                        blend(s->options->fg, s->options->bg, alpha));
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

    if (s->y < (s->options->y - s->options->font->font->line_height)) {
        s->y += s->options->font->font->line_height;
        return true;
    }

    if (s->options->bg != TEXT_BG_NONE) {
        int16_t width = mf_get_string_width(s->options->font->font, line, count, false) + 2 * s->options->margin;
        int16_t line_height = s->options->font->font->line_height;

        if (s->options->alignment == MF_ALIGN_LEFT) {
            lcd_write_rect(s->options->x,
                           MAX(s->y, s->options->y),
                           s->options->x + width,
                           MIN(s->y + line_height, s->options->y + s->options->height),
                           s->options->bg);
        } else if (s->options->alignment == MF_ALIGN_CENTER) {
            lcd_write_rect(s->options->x + s->options->width / 2 - width / 2,
                           MAX(s->y, s->options->y),
                           s->options->x + s->options->width / 2 + width / 2,
                           MIN(s->y + line_height, s->options->y + s->options->height),
                           s->options->bg);
        } else if (s->options->alignment == MF_ALIGN_RIGHT) {
            lcd_write_rect(s->options->x + s->options->width - width,
                           MAX(s->y, s->options->y),
                           s->options->x + s->options->width,
                           MIN(s->y + line_height, s->options->y + s->options->height),
                           s->options->bg);
        }
    }

    if (s->options->justify) {
        mf_render_justified(s->options->font->font, s->anchor + s->options->x, s->y,
                            s->options->width - s->options->margin * 2,
                            line, count, character_callback, state);
    } else {
        mf_render_aligned(s->options->font->font, s->anchor + s->options->x, s->y,
                          s->options->alignment, line, count,
                          character_callback, state);
    }

    s->y += s->options->font->font->line_height;
    return (s->y < (s->options->y + s->options->height))
            && (s->y < LCD_HEIGHT);
}

void text_prepare_font(struct text_font *tf) {
    if (!tf) {
        debug("invalid param");
        return;
    }

    debug("searching for \"%s\"", tf->fontname);

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

int16_t text_draw(struct text_conf *tc) {
    if ((!tc) || (!tc->font) || (!tc->font->font)) {
        debug("invalid param");
        return 0;
    }

    state_t state;
    state.options = tc;
    state.y = tc->y + tc->y_text_off;

    if (tc->alignment == MF_ALIGN_LEFT) {
        state.anchor = tc->margin;
    } else if (tc->alignment == MF_ALIGN_CENTER) {
        state.anchor = tc->width / 2;
    } else if (tc->alignment == MF_ALIGN_RIGHT) {
        state.anchor = tc->width - tc->margin;
    }

    mf_wordwrap(tc->font->font, tc->width - 2 * tc->margin,
                tc->text, line_callback, &state);

    return state.y;
}
