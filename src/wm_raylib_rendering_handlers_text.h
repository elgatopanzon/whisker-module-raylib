/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_handlers_text
 * @created     : Friday May 08, 2026 23:40:54 CST
 * @description : Text rendering handlers for 2D and 3D fonts using raw rlgl
 */

#ifndef WM_RAYLIB_RENDERING_HANDLERS_TEXT_H

#define WM_RAYLIB_RENDERING_HANDLERS_TEXT_H

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"
// GLAD is a desktop OpenGL loader - exclude for WASM which uses WebGL/GLES
#if defined(__EMSCRIPTEN__)
    #include <GLES3/gl3.h>
#else
    #include <external/glad.h>
#endif

#include <math.h>
#include <raymath.h>

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

static inline int wm_raylib_rendering_font_get_glyph_index(Font font, int codepoint)
{
    int index = 0;
    int fallbackIndex = 0;      // Get index of fallback glyph '?'

    /* Fast path: if codepoint falls in a contiguous range starting at
     * glyphs[0].value, compute the index directly in O(1).
     * Verify the hit to guard against non-contiguous layouts. */
    int base = font.glyphs[0].value;
    int fast = codepoint - base;
    if (fast >= 0 && fast < font.glyphCount && font.glyphs[fast].value == codepoint)
        return fast;

    // Look for character index in the unordered charset
    for (int i = 0; i < font.glyphCount; i++)
    {
        if (font.glyphs[i].value == 63) fallbackIndex = i;

        if (font.glyphs[i].value == codepoint)
        {
            index = i;
            break;
        }
    }

    if ((index == 0) && (font.glyphs[0].value != codepoint)) index = fallbackIndex;

    return index;
}

// Get next codepoint in a UTF-8 encoded text, scanning until '\0' is found
// When an invalid UTF-8 byte is encountered we exit as soon as possible and a '?'(0x3f) codepoint is returned
// Total number of bytes processed are returned as a parameter
// NOTE: The standard says U+FFFD should be returned in case of errors
// but that character is not supported by the default font in raylib
static inline int wm_raylib_rendering_font_get_codepoint(const char *text, int *codepointSize)
{
/*
    UTF-8 specs from https://www.ietf.org/rfc/rfc3629.txt

    Char. number range  |        UTF-8 octet sequence
      (hexadecimal)    |              (binary)
    --------------------+---------------------------------------------
    0000 0000-0000 007F | 0xxxxxxx
    0000 0080-0000 07FF | 110xxxxx 10xxxxxx
    0000 0800-0000 FFFF | 1110xxxx 10xxxxxx 10xxxxxx
    0001 0000-0010 FFFF | 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
*/
    // NOTE: on decode errors we return as soon as possible

    int codepoint = 0x3f;   // Codepoint (defaults to '?')
    int octet = (unsigned char)(text[0]); // The first UTF8 octet
    *codepointSize = 1;

    if (octet <= 0x7f)
    {
        // Only one octet (ASCII range x00-7F)
        codepoint = text[0];
    }
    else if ((octet & 0xe0) == 0xc0)
    {
        // Two octets

        // [0]xC2-DF    [1]UTF8-tail(x80-BF)
        unsigned char octet1 = text[1];

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if ((octet >= 0xc2) && (octet <= 0xdf))
        {
            codepoint = ((octet & 0x1f) << 6) | (octet1 & 0x3f);
            *codepointSize = 2;
        }
    }
    else if ((octet & 0xf0) == 0xe0)
    {
        // Three octets
        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; } // Unexpected sequence

        // [0]xE0    [1]xA0-BF       [2]UTF8-tail(x80-BF)
        // [0]xE1-EC [1]UTF8-tail    [2]UTF8-tail(x80-BF)
        // [0]xED    [1]x80-9F       [2]UTF8-tail(x80-BF)
        // [0]xEE-EF [1]UTF8-tail    [2]UTF8-tail(x80-BF)

        if (((octet == 0xe0) && !((octet1 >= 0xa0) && (octet1 <= 0xbf))) ||
            ((octet == 0xed) && !((octet1 >= 0x80) && (octet1 <= 0x9f)))) { *codepointSize = 2; return codepoint; }

        if ((octet >= 0xe0) && (octet <= 0xef))
        {
            codepoint = ((octet & 0xf) << 12) | ((octet1 & 0x3f) << 6) | (octet2 & 0x3f);
            *codepointSize = 3;
        }
    }
    else if ((octet & 0xf8) == 0xf0)
    {
        // Four octets
        if (octet > 0xf4) return codepoint;

        unsigned char octet1 = text[1];
        unsigned char octet2 = '\0';
        unsigned char octet3 = '\0';

        if ((octet1 == '\0') || ((octet1 >> 6) != 2)) { *codepointSize = 2; return codepoint; }  // Unexpected sequence

        octet2 = text[2];

        if ((octet2 == '\0') || ((octet2 >> 6) != 2)) { *codepointSize = 3; return codepoint; }  // Unexpected sequence

        octet3 = text[3];

        if ((octet3 == '\0') || ((octet3 >> 6) != 2)) { *codepointSize = 4; return codepoint; }  // Unexpected sequence

        // [0]xF0       [1]x90-BF       [2]UTF8-tail  [3]UTF8-tail
        // [0]xF1-F3    [1]UTF8-tail    [2]UTF8-tail  [3]UTF8-tail
        // [0]xF4       [1]x80-8F       [2]UTF8-tail  [3]UTF8-tail

        if (((octet == 0xf0) && !((octet1 >= 0x90) && (octet1 <= 0xbf))) ||
            ((octet == 0xf4) && !((octet1 >= 0x80) && (octet1 <= 0x8f)))) { *codepointSize = 2; return codepoint; } // Unexpected sequence

        if (octet >= 0xf0)
        {
            codepoint = ((octet & 0x7) << 18) | ((octet1 & 0x3f) << 12) | ((octet2 & 0x3f) << 6) | (octet3 & 0x3f);
            *codepointSize = 4;
        }
    }

    if (codepoint > 0x10ffff) codepoint = 0x3f;     // Codepoints after U+10ffff are invalid

    return codepoint;
}


/* Draw a single font codepoint (character) at a world position using rlgl quads.
 *
 * Supports both 2D (XY plane) and 3D (XZ plane) rendering. The active mode is
 * detected from GL_DEPTH_TEST: enabled = 3D, disabled = 2D.
 *
 * Glyph geometry:
 *   Each glyph lives in a font atlas texture at a known region (font.recs[index]).
 *   The atlas region is expanded by glyphPadding on all sides during atlas packing
 *   to prevent adjacent-glyph bleeding from bilinear filtering. srcRec includes
 *   this padding so the full padded region is sampled.
 *
 *   UV coordinates (tx/ty = top-left, tw/th = bottom-right) are computed as
 *   normalized [0..1] values into the atlas texture.
 *
 * Position offset logic:
 *   font.glyphs[index].offsetX/offsetY are per-glyph bearings -- the distance
 *   from the current pen position to where the glyph quad's top-left corner
 *   should sit. In 3D mode these are divided by baseSize and multiplied by
 *   scale to convert from font pixels to world units. In 2D they are just
 *   multiplied by scale (pixel space).
 *
 * Width/height in 3D vs 2D:
 *   3D: divided by baseSize first (world-unit normalization), then scaled.
 *   2D: multiplied by scale directly (stays in pixel space).
 *
 * Shadow pass:
 *   When shadow_offset is nonzero, the glyph is drawn twice: once at the
 *   shadow_offset position with shadow_color, then again at position with tint.
 *   Shadow is drawn at z-0.001 (2D) or y-0.001 (3D) to avoid z-fighting with
 *   the main glyph since they occupy the same plane.
 *   In 3D, shadow_offset is scaled by the same world-unit formula as glyph offsets.
 *
 * Backface in 3D:
 *   When backface is true, each quad is drawn twice -- once with normal pointing
 *   up (0,1,0) and once with normal pointing down (0,-1,0) -- so glyphs are
 *   visible from both sides of the XZ plane.
 *
 * Params:
 *   is_3d        - draw in 3D (world) or 2D (screen)
 *   font         - raylib Font to sample glyphs from (atlas texture + glyph metrics)
 *   codepoint    - Unicode codepoint of the character to draw
 *   position     - world-space pen position (glyph offset applied internally)
 *   font_size    - desired render size in pixels (2D) or world units (3D)
 *   backface     - if true, draw the quad from both sides (3D only)
 *   tint         - foreground color for the glyph
 *   shadow_offset - pixel/world offset for the shadow pass; (0,0) disables shadow
 *   shadow_color  - color for the shadow pass
 */
static inline void wm_raylib_rendering_draw_font_codepoint(
	bool is_3d,
    Font font,
    int codepoint,
    int index,
    w_vec3 position,
    float font_size,
    bool backface,
    w_color8 tint,
    w_vec2 shadow_offset,
    w_color8 shadow_color
)
{
    /* scale converts from font base pixels to the desired render size.
     * e.g. font.baseSize=32, font_size=64 -> scale=2.0 (double size) */
    float scale = font_size / (float)font.baseSize;

    /* srcRec is the region in the atlas texture for this glyph, expanded by
     * glyphPadding on all sides. The padding is needed because raylib packs
     * glyphs with extra border pixels to avoid bleeding when sampled with
     * bilinear filtering at sub-pixel positions. */
    w_rect src_rec = {
        font.recs[index].x - (float)font.glyphPadding,
        font.recs[index].y - (float)font.glyphPadding,
        font.recs[index].width  + 2.0f * font.glyphPadding,
        font.recs[index].height + 2.0f * font.glyphPadding
    };

    /* Only enable the shadow pass if the offset is nonzero on either axis. */
    bool has_shadow = (shadow_offset.x != 0.0f || shadow_offset.y != 0.0f);

    /* Compute the padded quad dimensions and apply the glyph bearing offset to
     * position. The bearing moves the pen to the correct quad origin for this
     * specific glyph (characters like 'g' hang below the baseline, 'A' sits
     * above, etc.).
     *
     * 3D: divide by baseSize first to normalize to world units, then scale.
     * 2D: scale directly in pixel space. */
    float width, height;
    if (is_3d)
    {
        position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)
                      / (float)font.baseSize * scale;
        position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)
                      / (float)font.baseSize * scale;
        width  = (float)(font.recs[index].width  + 2.0f * font.glyphPadding)
                 / (float)font.baseSize * scale;
        height = (float)(font.recs[index].height + 2.0f * font.glyphPadding)
                 / (float)font.baseSize * scale;
    }
    else
    {
        position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding) * scale;
        position.y += (float)(font.glyphs[index].offsetY - font.glyphPadding) * scale;
        width  = (float)(font.recs[index].width  + 2.0f * font.glyphPadding) * scale;
        height = (float)(font.recs[index].height + 2.0f * font.glyphPadding) * scale;
    }

    if (font.texture.id > 0)
    {
        /* Local quad origin is always at (0,0,0); the matrix stack handles
         * world position. This keeps the vertex math simple. */
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        /* Normalize atlas region to [0..1] UV coordinates.
         * tx/ty = top-left UV corner, tw/th = bottom-right UV corner. */
        const float tx = src_rec.x / font.texture.width;
        const float ty = src_rec.y / font.texture.height;
        const float tw = (src_rec.x + src_rec.width)  / font.texture.width;
        const float th = (src_rec.y + src_rec.height) / font.texture.height;

        /* Reserve enough space in the render batch for both passes.
         * Each quad = 4 verts. 3D backface doubles the verts per pass.
         * (1 + has_shadow) doubles everything when shadow is active. */
        rlCheckRenderBatchLimit((4 + 4 * (is_3d && backface)) * (1 + has_shadow));
        rlSetTexture(font.texture.id);

        /* Shadow pass: draw the glyph at the offset position in shadow_color,
         * placed slightly behind the main glyph on the depth axis to prevent
         * z-fighting (0.001 world units / depth units below main glyph). */
        if (has_shadow)
        {
            rlPushMatrix();
                /* 3D: shadow_offset is in pixel units relative to font_size, so
                 * apply the same world-unit scaling used for glyph positions.
                 * Divide by 2 because the offset x/y map to XZ not to the full
                 * baseSize range.
                 * 2D: shadow_offset is in screen pixels, used directly. */
                if (is_3d)
                    rlTranslatef(
                        position.x + shadow_offset.x / 2.0f / (float)font.baseSize * scale,
                        position.y - 0.001f,
                        position.z + shadow_offset.y / 2.0f / (float)font.baseSize * scale
                    );
                else
                    rlTranslatef(
                        position.x + shadow_offset.x,
                        position.y + shadow_offset.y,
                        position.z - 0.001f
                    );

                rlBegin(RL_QUADS);
                    rlColor4ub(shadow_color.r, shadow_color.g, shadow_color.b, shadow_color.a);

                    if (is_3d)
                    {
                        /* Front face: normal pointing up (+Y) for top-view visibility. */
                        rlNormal3f(0.0f, 1.0f, 0.0f);
                        rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);
                        rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);
                        rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);
                        rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);

                        if (backface)
                        {
                            /* Back face: reverse winding + flipped normal for
                             * under-plane visibility. */
                            rlNormal3f(0.0f, -1.0f, 0.0f);
                            rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);
                            rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);
                            rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);
                            rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);
                        }
                    }
                    else
                    {
                        /* 2D: XY plane, normal points towards the camera (+Z). */
                        rlNormal3f(0.0f, 0.0f, 1.0f);
                        rlTexCoord2f(tx, ty); rlVertex3f(x,         y,          0.0f);
                        rlTexCoord2f(tx, th); rlVertex3f(x,         y + height, 0.0f);
                        rlTexCoord2f(tw, th); rlVertex3f(x + width, y + height, 0.0f);
                        rlTexCoord2f(tw, ty); rlVertex3f(x + width, y,          0.0f);
                    }
                rlEnd();
            rlPopMatrix();
        }

        /* Main text pass: draw the glyph at position with tint. */
        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                if (is_3d)
                {
                    rlNormal3f(0.0f, 1.0f, 0.0f);
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);

                    if (backface)
                    {
                        rlNormal3f(0.0f, -1.0f, 0.0f);
                        rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);
                        rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);
                        rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);
                        rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);
                    }
                }
                else
                {
                    rlNormal3f(0.0f, 0.0f, 1.0f);
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y,          0.0f);
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y + height, 0.0f);
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y + height, 0.0f);
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y,          0.0f);
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

/* Measure the pixel/world width of a single word starting at text[0].
 * A word ends at the first whitespace character (' ', '\t', '\n') or null.
 *
 * This is used as a look-ahead during word-wrap: before advancing past a space,
 * we measure the next word to decide if it still fits on the current line.
 *
 * Advance width per glyph:
 *   - If advanceX == 0, raylib falls back to rect width (no explicit advance set).
 *   - Otherwise advanceX is the author-specified horizontal advance.
 *   - In 3D: advance is divided by baseSize to normalize to world units, then scaled.
 *   - In 2D: advance is multiplied by scale directly (pixel space).
 *   spacing is added to each glyph's advance regardless of mode.
 *
 * Params:
 *   font       - font providing glyph metrics
 *   text       - pointer to start of the word (not necessarily start of string)
 *   font_size  - render font size (pixels or world units)
 *   scale      - precomputed font_size / font.baseSize
 *   spacing    - extra horizontal space added after each glyph
 *   is_3d      - true = 3D world-unit formula, false = 2D pixel formula
 *   byte_count - if non-NULL, receives the number of bytes consumed (excl. whitespace)
 *
 * Returns: total advance width of the word
 */
static inline float wm_raylib_rendering_measure_word_width(
    Font font,
    const char *text,
    float font_size,
    float scale,
    float spacing,
    bool is_3d,
    int *byte_count
)
{
    float width = 0.0f;
    int bytes = 0;

    for (int i = 0; text[i] != '\0';)
    {
        int codepoint_byte_count = 0;
        int codepoint = wm_raylib_rendering_font_get_codepoint(&text[i], &codepoint_byte_count);

        /* wm_raylib_rendering_font_get_codepoint returns 0x3f ('?') for invalid byte sequences.
         * Force advance by 1 byte to avoid an infinite loop on bad UTF-8. */
        if (codepoint == 0x3f) codepoint_byte_count = 1;

        /* Stop at any whitespace -- that's the word boundary. */
        if (codepoint == ' ' || codepoint == '\t' || codepoint == '\n')
            break;

        int index = wm_raylib_rendering_font_get_glyph_index(font, codepoint);

        /* Accumulate advance. Use rect width as fallback when advanceX == 0. */
        if (font.glyphs[index].advanceX == 0)
            width += is_3d
                ? (float)(font.recs[index].width + spacing) / (float)font.baseSize * scale
                : (float)font.recs[index].width * scale + spacing;
        else
            width += is_3d
                ? (float)(font.glyphs[index].advanceX + spacing) / (float)font.baseSize * scale
                : (float)font.glyphs[index].advanceX * scale + spacing;

        i     += codepoint_byte_count;
        bytes += codepoint_byte_count;
    }

    if (byte_count) *byte_count = bytes;
    return width;
}

/* Measure the pixel/world width of a single line starting at text[0].
 * A line ends at the first '\n' or null terminator.
 * The '\n' byte itself is included in *byte_count but not in the width.
 *
 * This is used to compute alignment offsets before rendering each line:
 * center alignment needs half the line width, right alignment needs the full width.
 *
 * Params:
 *   font       - font providing glyph metrics
 *   text       - pointer to start of the line
 *   font_size  - render font size
 *   scale      - precomputed font_size / font.baseSize
 *   spacing    - extra horizontal space added after each glyph
 *   is_3d      - 3D world-unit formula vs 2D pixel formula
 *   byte_count - if non-NULL, receives bytes consumed (including the '\n' if present)
 *
 * Returns: rendered width of the line (not accounting for wrapping)
 */
static inline float wm_raylib_rendering_measure_line_width(
    Font font,
    const char *text,
    float font_size,
    float scale,
    float spacing,
    bool is_3d,
    int *byte_count
)
{
    float width = 0.0f;
    int bytes = 0;

    for (int i = 0; text[i] != '\0';)
    {
        int codepoint_byte_count = 0;
        int codepoint = wm_raylib_rendering_font_get_codepoint(&text[i], &codepoint_byte_count);
        if (codepoint == 0x3f) codepoint_byte_count = 1;

        /* Stop at newline; count the newline byte but don't add its width. */
        if (codepoint == '\n')
        {
            bytes += codepoint_byte_count;
            break;
        }

        int index = wm_raylib_rendering_font_get_glyph_index(font, codepoint);
        if (font.glyphs[index].advanceX == 0)
            width += is_3d
                ? (float)(font.recs[index].width + spacing) / (float)font.baseSize * scale
                : (float)font.recs[index].width * scale + spacing;
        else
            width += is_3d
                ? (float)(font.glyphs[index].advanceX + spacing) / (float)font.baseSize * scale
                : (float)font.glyphs[index].advanceX * scale + spacing;

        i     += codepoint_byte_count;
        bytes += codepoint_byte_count;
    }

    if (byte_count) *byte_count = bytes;
    return width;
}

/* Measure the width of the segment that will actually render before a wrap occurs.
 *
 * Unlike measure_line_width (which ignores max_width), this function simulates
 * the same wrap decisions that rm_raylib_rendering_draw_text will make:
 *   - At each space, look ahead at the next word. If adding space+word would
 *     exceed max_width, stop here (soft wrap at word boundary).
 *   - For non-space glyphs, if adding the glyph would exceed max_width and we're
 *     not at the start of a line, stop here (hard wrap mid-word).
 *
 * This width is used to compute alignment offsets for wrapped lines, so centered
 * and right-aligned text wraps correctly.
 *
 * If max_width <= 0 or no wrap point is reached, returns the full line width up
 * to the next '\n' or end of string (same as measure_line_width).
 *
 * Params:
 *   font      - font providing glyph metrics
 *   text      - pointer to start of the line segment to measure
 *   font_size - render font size
 *   scale     - precomputed font_size / font.baseSize
 *   spacing   - extra horizontal space per glyph
 *   is_3d     - 3D world-unit formula vs 2D pixel formula
 *   max_width - wrap boundary (0 = disabled)
 *
 * Returns: width that will be rendered on this line before wrapping
 */
static inline float wm_raylib_rendering_measure_wrapped_line_width(
    Font font,
    const char *text,
    float font_size,
    float scale,
    float spacing,
    bool is_3d,
    float max_width
)
{
    float width = 0.0f;

    /* pre_space_width: width before the most recent space -- the soft-wrap
     * return point. Updated at every space so it always tracks the last one.
     *
     * word_after_space: true once we have seen at least one space on this
     * line. When an overflow occurs inside a word that follows a space we do
     * a soft wrap (return pre_space_width). When an overflow occurs inside
     * the very first word (no prior space) we do a hard wrap (return width).
     *
     * This single-pass approach is equivalent to the previous lookahead:
     * the original checked whether the entire next word fits at each space;
     * here we process the next word's chars greedily and return pre_space_width
     * the moment any char overflows -- which fires at the same boundary. */
    float pre_space_width = 0.0f;
    bool at_line_start = true;
    bool word_after_space = false;

    for (int i = 0; text[i] != '\0';)
    {
        int codepoint_byte_count = 0;
        int codepoint = wm_raylib_rendering_font_get_codepoint(&text[i], &codepoint_byte_count);
        if (codepoint == 0x3f) codepoint_byte_count = 1;

        if (codepoint == '\n')
            break;

        int index = wm_raylib_rendering_font_get_glyph_index(font, codepoint);
        float char_width = 0.0f;
        if (font.glyphs[index].advanceX == 0)
            char_width = is_3d
                ? (float)(font.recs[index].width + spacing) / (float)font.baseSize * scale
                : (float)font.recs[index].width * scale + spacing;
        else
            char_width = is_3d
                ? (float)(font.glyphs[index].advanceX + spacing) / (float)font.baseSize * scale
                : (float)font.glyphs[index].advanceX * scale + spacing;

        if (codepoint == ' ' || codepoint == '\t')
        {
            /* Record the width before this space as the last safe soft-wrap
             * point, then advance. */
            pre_space_width = width;
            word_after_space = true;
            width += char_width;
            at_line_start = false;
        }
        else
        {
            if (max_width > 0.0f && width + char_width > max_width && !at_line_start)
            {
                /* Soft wrap: overflow in a word that follows a space -- return
                 * the width before that space (trailing space not counted).
                 * Hard wrap: overflow in the first word on the line -- return
                 * the current width (hyphen will be placed at this position). */
                return word_after_space ? pre_space_width : width;
            }
            width += char_width;
            at_line_start = false;
        }

        i += codepoint_byte_count;
    }

    return width;
}

/* Draw a UTF-8 string using the unified 2D/3D rendering path.
 *
 * Mode detection:
 *   GL_DEPTH_TEST enabled = 3D (XZ plane, world units).
 *   GL_DEPTH_TEST disabled = 2D (XY plane, pixels).
 *   This is checked once and cached in is_3d for the entire call.
 *
 * Coordinate conventions:
 *   3D: text lays out on the XZ plane. X = horizontal advance, Z = line descent.
 *       Y is the world-space height of the text plane (unchanged during layout).
 *   2D: text lays out in standard screen space. X = horizontal, Y = line descent.
 *
 * Line advance (vertical spacing between lines):
 *   3D: (scale + 1/baseSize * scale) -- one em in world units plus a small gap.
 *   2D: (font_size + 2) -- font size in pixels plus a fixed 2px gap.
 *   line_height multiplies this base advance (1.0 = default, 1.5 = 50% extra, etc.)
 *
 * Alignment:
 *   For each line (explicit '\n' or word-wrapped), the full line width is measured
 *   first, then an alignment offset is computed:
 *   - LEFT:   offset = 0 (no shift)
 *   - CENTER: offset = -lineWidth / 2  (shift left by half)
 *   - RIGHT:  offset = -lineWidth      (shift left by full)
 *   textOffsetX starts at this alignment offset and advances rightward per glyph.
 *
 * Word wrapping (max_width > 0):
 *   At each space character, measure_word_width looks ahead at the next word.
 *   If current_line_width + space + next_word > max_width, the space is consumed
 *   as a newline: advance Y (or Z in 3D), reset offsets, skip the space.
 *
 *   If a single word exceeds max_width mid-word, a '-' hyphen is drawn at the
 *   wrap point, then the renderer wraps to the next line and re-processes the
 *   current character. The hyphen signals that the break is forced, not natural.
 *
 * Params:
 *   is_3d        - render in 3D (world) or 2D (screen)
 *   font         - raylib Font to render with
 *   text         - null-terminated UTF-8 string
 *   position     - world-space top-left origin for the text block
 *   font_size    - desired render size (pixels in 2D, world units in 3D before
 *                  the caller converts via W_RENDERING_WORLD_PIXEL_SCALE)
 *   spacing      - extra horizontal space per glyph
 *   line_height  - line spacing multiplier (0 or 1.0 = default, 2.0 = double)
 *   max_width    - word-wrap boundary; 0 disables wrapping
 *   backface     - draw glyphs on both sides of the plane (3D only)
 *   tint         - foreground text color
 *   shadow_offset - drop-shadow pixel offset; (0,0) disables shadow
 *   shadow_color  - drop-shadow color
 *   align        - horizontal alignment (LEFT, CENTER, RIGHT)
 */
static inline void rm_raylib_rendering_draw_text(
	bool is_3d,
    Font font,
    const char *text,
    w_vec3 position,
    float font_size,
    float spacing,
    float line_height,
    float max_width,
    bool backface,
    w_color8 tint,
    w_vec2 shadow_offset,
    w_color8 shadow_color,
    enum W_RENDERING_TEXT_ALIGN align
)
{
    int length = TextLength(text);

    /* text_offset_x tracks the horizontal pen position relative to position.x.
     * text_offset_y tracks vertical descent per line (in the Y or Z axis). */
    float text_offset_y = 0.0f;
    float text_offset_x = 0.0f;

    /* scale = render size / base pixel size of the font atlas.
     * All glyph metrics are stored in atlas pixels; multiplying by scale gives
     * the final rendered size in pixels (2D) or world units (3D). */
    float scale = font_size / (float)font.baseSize;

    /* Normalize line_height: 0 is treated as 1.0 (no override). */
    if (line_height == 0) line_height = 1.0f;

    /* Compute alignment offset for the first line before the main loop.
     * Skipped for LEFT alignment since the offset is always 0.
     * measure_line_width returns the full rendered width up to the first '\n'.
     * For word-wrapped text this may overestimate the actual first-line width,
     * but it's corrected whenever the wrapped-line path runs. */
    float measured_line_width = 0.0f;
    if (align != W_RENDERING_TEXT_ALIGN_LEFT)
        measured_line_width = wm_raylib_rendering_measure_line_width(
            font, text, font_size, scale, spacing, is_3d, NULL
        );
    float align_offset = 0.0f;
    if (align == W_RENDERING_TEXT_ALIGN_CENTER)
        align_offset = -measured_line_width / 2.0f;
    else if (align == W_RENDERING_TEXT_ALIGN_RIGHT)
        align_offset = -measured_line_width;
    text_offset_x = align_offset;

    /* line_width accumulates the rendered width of the current line for wrap
     * decisions. Reset to 0 on every newline or wrap event. */
    float line_width = 0.0f;

    /* at_line_start prevents wrapping before any glyph has been placed on a
     * new line. Without this, an overlong word at the start would loop forever
     * trying to force-break before anything is drawn. */
    bool at_line_start = true;

    /* line_advance is the base vertical distance between lines before the
     * line_height multiplier is applied.
     * 3D: one em in world units (scale) + a small rounding gap (1/baseSize*scale).
     * 2D: font_size in pixels + 2px fixed gap (raylib convention). */
    float line_advance = is_3d
        ? scale + 1.0f / (float)font.baseSize * scale
        : font_size + 2;

    for (int i = 0; i < length;)
    {
        int codepoint_byte_count = 0;
        int codepoint = wm_raylib_rendering_font_get_codepoint(&text[i], &codepoint_byte_count);
        int index = wm_raylib_rendering_font_get_glyph_index(font, codepoint);

        /* Bad byte sequences return 0x3f ('?'). Advance by 1 byte to avoid
         * getting stuck on invalid UTF-8 input. */
        if (codepoint == 0x3f) codepoint_byte_count = 1;

        /* Compute this glyph's horizontal advance (in pixels or world units). */
        float char_width = 0.0f;
        if (font.glyphs[index].advanceX == 0)
            char_width = is_3d
                ? (float)(font.recs[index].width + spacing) / (float)font.baseSize * scale
                : (float)font.recs[index].width * scale + spacing;
        else
            char_width = is_3d
                ? (float)(font.glyphs[index].advanceX + spacing) / (float)font.baseSize * scale
                : (float)font.glyphs[index].advanceX * scale + spacing;

        if (codepoint == '\n')
        {
            /* Explicit newline: advance the line, compute alignment for next line. */
            text_offset_y += line_advance * line_height;

            /* Measure from the character after '\n' to get the next line's width.
             * Skipped for LEFT alignment since the offset is always 0. */
            measured_line_width = 0.0f;
            if (align != W_RENDERING_TEXT_ALIGN_LEFT)
                measured_line_width = wm_raylib_rendering_measure_line_width(
                    font, &text[i + codepoint_byte_count], font_size, scale, spacing, is_3d, NULL
                );
            align_offset = 0.0f;
            if (align == W_RENDERING_TEXT_ALIGN_CENTER)
                align_offset = -measured_line_width / 2.0f;
            else if (align == W_RENDERING_TEXT_ALIGN_RIGHT)
                align_offset = -measured_line_width;
            text_offset_x = align_offset;
            line_width = 0.0f;
            at_line_start = true;
        }
        else if (codepoint == ' ' || codepoint == '\t')
        {
            /* Space/tab: before advancing, check if the next word would overflow.
             * If yes, this space becomes a soft newline (consumed, not drawn). */
            if (max_width > 0.0f && !at_line_start)
            {
                float word_width = wm_raylib_rendering_measure_word_width(
                    font, &text[i + codepoint_byte_count], font_size, scale, spacing, is_3d, NULL
                );
                if (line_width + char_width + word_width > max_width)
                {
                    /* Soft wrap: skip this space, drop to next line. */
                    text_offset_y += line_advance * line_height;

                    /* Measure the wrapped line width from the next word for
                     * correct alignment on the new line. Skipped for LEFT. */
                    float next_line_width = 0.0f;
                    if (align != W_RENDERING_TEXT_ALIGN_LEFT)
                        next_line_width = wm_raylib_rendering_measure_wrapped_line_width(
                            font, &text[i + codepoint_byte_count], font_size, scale, spacing, is_3d, max_width
                        );
                    align_offset = 0.0f;
                    if (align == W_RENDERING_TEXT_ALIGN_CENTER)
                        align_offset = -next_line_width / 2.0f;
                    else if (align == W_RENDERING_TEXT_ALIGN_RIGHT)
                        align_offset = -next_line_width;
                    text_offset_x = align_offset;
                    line_width = 0.0f;
                    at_line_start = true;
                    i += codepoint_byte_count;
                    continue;
                }
            }
            /* Space fits on current line: advance pen but draw nothing. */
            text_offset_x += char_width;
            line_width    += char_width;
            at_line_start  = false;
        }
        else
        {
            /* Regular glyph: check for hard (mid-word) overflow. */
            if (max_width > 0.0f && line_width + char_width > max_width && !at_line_start)
            {
                /* Force break: draw a '-' at the current position to signal the
                 * break, then wrap. The current character is re-processed on the
                 * new line (continue without advancing i). */
                w_vec3 hyphen_pos = is_3d
                    ? (w_vec3){ position.x + text_offset_x, position.y, position.z + text_offset_y }
                    : (w_vec3){ position.x + text_offset_x, position.y + text_offset_y, 0.0f };
                wm_raylib_rendering_draw_font_codepoint(
                    is_3d, font, '-', index, hyphen_pos, font_size, backface, tint, shadow_offset, shadow_color
                );

                text_offset_y += line_advance * line_height;

                /* Measure from the current character (not the next word) since
                 * this character will start the new line. Skipped for LEFT. */
                float next_line_width = 0.0f;
                if (align != W_RENDERING_TEXT_ALIGN_LEFT)
                    next_line_width = wm_raylib_rendering_measure_wrapped_line_width(
                        font, &text[i], font_size, scale, spacing, is_3d, max_width
                    );
                align_offset = 0.0f;
                if (align == W_RENDERING_TEXT_ALIGN_CENTER)
                    align_offset = -next_line_width / 2.0f;
                else if (align == W_RENDERING_TEXT_ALIGN_RIGHT)
                    align_offset = -next_line_width;
                text_offset_x = align_offset;
                line_width = 0.0f;
                at_line_start = true;
                /* Re-process the current character on the new line. */
                continue;
            }

            /* Normal case: place glyph at current pen position. */
            w_vec3 pos = is_3d
                ? (w_vec3){ position.x + text_offset_x, position.y,              position.z + text_offset_y }
                : (w_vec3){ position.x + text_offset_x, position.y + text_offset_y, 0.0f };
            wm_raylib_rendering_draw_font_codepoint(
                is_3d, font, codepoint, index, pos, font_size, backface, tint, shadow_offset, shadow_color
            );

            text_offset_x += char_width;
            line_width    += char_width;
            at_line_start  = false;
        }

        i += codepoint_byte_count;
    }
}


/* ECS render command handler for W_RENDERING_CMD_DRAW_TEXT.
 *
 * Reads a w_rendering_cmd_draw_text payload from the render command queue and
 * draws the text string using rm_raylib_rendering_draw_text.
 *
 * 2D mode fixup:
 *   Raylib's 2D (screen-mode) camera uses a non-standard coordinate layout.
 *   Incoming commands use the XZ/-Y convention used by the 3D world camera.
 *   For 2D we apply a +90 degree X rotation to cancel the -90 degree that the
 *   camera applies, and swap Y/Z on position, scale, and origin so the text
 *   ends up in the correct screen position.
 *
 * Matrix composition (applied via rlgl matrix stack):
 *   1. Translate to -origin   (shift pivot to local origin)
 *   2. Rotate + scale         (apply rotation quaternion and scale)
 *   3. Translate to position  (move to world/screen position)
 *   This order means origin is the pivot point for rotation and scale.
 *
 * World-pixel scale (3D only):
 *   font_size and max_width are stored in the command as logical pixel counts.
 *   In 3D, they must be converted to world units by multiplying by baseSize and
 *   dividing by W_RENDERING_WORLD_PIXEL_SCALE. This aligns font pixel sizes with
 *   the world grid used by all other 3D rendering.
 *
 * The local-space origin passed to rm_raylib_rendering_draw_text is always
 * (0,0,0) because the world position is already encoded in the matrix stack.
 *
 * Params:
 *   world   - ECS world (for camera/render config state)
 *   payload - pointer to w_rendering_cmd_draw_text
 */
static inline void wm_raylib_rendering_handle_draw_text(
	struct w_ecs_world *world,
	void *payload
)
{
	struct w_rendering_cmd_draw_text *cmd = payload;

	struct w_rendering_camera_state *camera_state = w_rendering_get_camera_state(world);
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);


	Font default_font = GetFontDefault();
	if (default_font.texture.id != 0)
	{
		rlPushMatrix();

		bool is_3d = glIsEnabled(GL_DEPTH_TEST);

		/* if not 3D, re-write for raylibs odd screen mode camera
		 * note: this is mostly to keep the incoming XZ and -Z convention for
		 * both world (camera) and overlay (screen) phases */
		if (!is_3d)
		{
			/* rotate +90 degrees on X to negate the incoming -90 degrees on X */
			cmd->rotation = w_quat_mul(cmd->rotation, w_quat_rotation_x_deg(90.0f));

			/* swap Z and Y for position and scale and origin */
			float old_pos_y = cmd->position.y;
			cmd->position.y = cmd->position.z;
			cmd->position.z = old_pos_y;

			float old_scale_y = cmd->scale.y;
			cmd->scale.y = cmd->scale.z;
			cmd->scale.z = old_scale_y;

			float old_origin_y = cmd->origin.y;
			cmd->origin.y = cmd->origin.z;
			cmd->origin.z = old_origin_y;
		}

		/* build matrix without position */
		w_mat4 rs = w_mat4_from_trs(
			((w_vec3){0, 0, 0}),
			cmd->rotation,
			cmd->scale
		);

		rlTranslatef(cmd->position.x, cmd->position.y, cmd->position.z);
		rlMultMatrixf(rs.m);
		rlTranslatef(-cmd->origin.x, -cmd->origin.y, -cmd->origin.z);

		/* scale font size and max width to world pixel scale */
		if (is_3d)
		{
			float base_size = (float)default_font.baseSize;
			cmd->font_size = ((float)cmd->font_size * base_size) / W_RENDERING_WORLD_PIXEL_SCALE;
			cmd->max_width = cmd->max_width / W_RENDERING_WORLD_PIXEL_SCALE;
		}

		/* draw the text */
		rm_raylib_rendering_draw_text(
			is_3d,
			default_font,
			cmd->text,
			((w_vec3){ 0.0f, 0.0f, 0.0f }),
			cmd->font_size,
			cmd->font_spacing,
			cmd->font_line_height,
			cmd->max_width,
			false,
			cmd->font_color,
			cmd->font_shadow,
			cmd->font_shadow_color,
			cmd->font_alignment
		);

		rlPopMatrix();
	}
}

#endif /* end of include guard WM_RAYLIB_RENDERING_HANDLERS_TEXT_H */
