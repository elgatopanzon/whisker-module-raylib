/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_handlers_draw
 * @created     : Sunday May 03, 2026 20:13:09 CST
 * @description : Render command handlers - draw begin/end, framebuffer, text, verts
 */

#ifndef WM_RAYLIB_RENDERING_HANDLERS_DRAW_H
#define WM_RAYLIB_RENDERING_HANDLERS_DRAW_H

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"
#include "external/glad.h"
#include <math.h>
#include <raymath.h>

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

static inline void wm_raylib_rendering_handle_draw_begin(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world; (void)payload;
	BeginDrawing();
}

static inline void wm_raylib_rendering_handle_draw_end(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world; (void)payload;
	EndDrawing();
}

static inline void wm_raylib_rendering_handle_draw_main_framebuffer(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)payload;
	struct w_rendering_render_state *render_state = w_rendering_get_render_state(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);
	// draw main framebuffer texture, flipping Y source height
	Rectangle rl_source = w2rl_rect(render_state->render_texture_source);
	rl_source.height = -rl_source.height;
	DrawTexturePro(raylib_render_state->render_texture.texture, rl_source, w2rl_rect(render_state->render_texture_destination), w2rl_vec2(render_state->draw_origin), render_state->draw_rotation, WHITE);
}


static void DrawTextCodepoint3D(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint)
{
    // Character index position in sprite font
    // NOTE: In case a codepoint is not available in the font, index returned points to '?'
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    // Character destination rectangle on screen
    // NOTE: We consider charsPadding on drawing
    position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)/(float)font.baseSize*scale;
    position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)/(float)font.baseSize*scale;

    // Character source rectangle from font texture atlas
    // NOTE: We consider chars padding when drawing, it could be required for outline/glow shader effects
    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    float width = (float)(font.recs[index].width + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
    float height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        // normalized texture coordinates of the glyph inside the font texture (0.0f -> 1.0f)
        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        rlCheckRenderBatchLimit(4 + 4*backface);
        rlSetTexture(font.texture.id);

        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                // Front Face
                rlNormal3f(0.0f, 1.0f, 0.0f);                                   // Normal Pointing Up
                rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);              // Top Left Of The Texture and Quad
                rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height);     // Bottom Left Of The Texture and Quad
                rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height);     // Bottom Right Of The Texture and Quad
                rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);              // Top Right Of The Texture and Quad

                if (backface)
                {
                    // Back Face
                    rlNormal3f(0.0f, -1.0f, 0.0f);                              // Normal Pointing Down
                    rlTexCoord2f(tx, ty); rlVertex3f(x,         y, z);          // Top Right Of The Texture and Quad
                    rlTexCoord2f(tw, ty); rlVertex3f(x + width, y, z);          // Top Left Of The Texture and Quad
                    rlTexCoord2f(tw, th); rlVertex3f(x + width, y, z + height); // Bottom Left Of The Texture and Quad
                    rlTexCoord2f(tx, th); rlVertex3f(x,         y, z + height); // Bottom Right Of The Texture and Quad
                }
            rlEnd();
        rlPopMatrix();

        rlSetTexture(0);
    }
}

// Draw a 2D text in 3D space
static void DrawText3D(Font font, const char *text, Vector3 position, float fontSize, float fontSpacing, float lineSpacing, bool backface, Color tint)
{
    int length = TextLength(text);          // Total length in bytes of the text, scanned by codepoints in loop

    float textOffsetY = 0.0f;               // Offset between lines (on line break '\n')
    float textOffsetX = 0.0f;               // Offset X to next character to draw

    float scale = fontSize/(float)font.baseSize;

    for (int i = 0; i < length;)
    {
        // Get next codepoint from byte string and glyph index in font
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            // NOTE: Fixed line spacing of 1.5 line-height
            // TODO: Support custom line spacing defined by user
            textOffsetY += scale + lineSpacing/(float)font.baseSize*scale;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                DrawTextCodepoint3D(font, codepoint, (Vector3){ position.x + textOffsetX, position.y, position.z + textOffsetY }, fontSize, backface, tint);
            }

            if (font.glyphs[index].advanceX == 0) textOffsetX += (float)(font.recs[index].width + fontSpacing)/(float)font.baseSize*scale;
            else textOffsetX += (float)(font.glyphs[index].advanceX + fontSpacing)/(float)font.baseSize*scale;
        }

        i += codepointByteCount;   // Move text bytes counter to next codepoint
    }
}

// Measure a text in 3D. For some reason `MeasureTextEx()` just doesn't seem to work so i had to use this instead.
static Vector3 MeasureText3D(Font font, const char* text, float fontSize, float fontSpacing, float lineSpacing)
{
    int len = TextLength(text);
    int tempLen = 0;                // Used to count longer text line num chars
    int lenCounter = 0;

    float tempTextWidth = 0.0f;     // Used to count longer text line width

    float scale = fontSize/(float)font.baseSize;
    float textHeight = scale;
    float textWidth = 0.0f;

    int letter = 0;                 // Current character
    int index = 0;                  // Index position in sprite font

    for (int i = 0; i < len; i++)
    {
        lenCounter++;

        int next = 0;
        letter = GetCodepoint(&text[i], &next);
        index = GetGlyphIndex(font, letter);

        // NOTE: normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol so to not skip any we set next = 1
        if (letter == 0x3f) next = 1;
        i += next - 1;

        if (letter != '\n')
        {
            if (font.glyphs[index].advanceX != 0) textWidth += (font.glyphs[index].advanceX+fontSpacing)/(float)font.baseSize*scale;
            else textWidth += (font.recs[index].width + font.glyphs[index].offsetX)/(float)font.baseSize*scale;
        }
        else
        {
            if (tempTextWidth < textWidth) tempTextWidth = textWidth;
            lenCounter = 0;
            textWidth = 0.0f;
            textHeight += scale + lineSpacing/(float)font.baseSize*scale;
        }

        if (tempLen < lenCounter) tempLen = lenCounter;
    }

    if (tempTextWidth < textWidth) tempTextWidth = textWidth;

    Vector3 vec = { 0 };
    vec.x = tempTextWidth + (float)((tempLen - 1)*fontSpacing/(float)font.baseSize*scale); // Adds chars spacing to measure
    vec.y = 0.25f;
    vec.z = textHeight;

    return vec;
}


static inline void wm_raylib_rendering_handle_draw_text(
	struct w_ecs_world *world,
	void *payload
)
{
	struct w_rendering_cmd_draw_text *cmd = payload;

	struct w_rendering_camera_state *camera_state = w_rendering_get_camera_state(world);
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);


	if (GetFontDefault().texture.id != 0)
	{
		/* int defaultFontSize = 10;   // default Font chars height in pixel */
		/* if (cmd->font_size < defaultFontSize) cmd->font_size = defaultFontSize; */
		/* int spacing = cmd->font_size/defaultFontSize; */

		// extract Y rotation angle from quat for 2D spin
		float yaw = w_quat_to_euler(cmd->rotation).y;
		rlPushMatrix();

			if (glIsEnabled(GL_DEPTH_TEST))
			{
				w_mat4 rs = w_mat4_from_trs(
					((w_vec3){0, 0, 0}),  // no position in matrix
					cmd->rotation,
					((w_vec3){1, 1, 1})
				);

				rlTranslatef(cmd->position.x, cmd->position.y, cmd->position.z);
				rlMultMatrixf(rs.m);
				rlTranslatef(-cmd->origin.x, -cmd->origin.y, -cmd->origin.z);

				// get world size from pixel scale
				float base_size = (float)GetFontDefault().baseSize;
				float world_font_size = ((float)cmd->font_size * base_size) / W_RENDERING_WORLD_PIXEL_SCALE;

				DrawText3D(GetFontDefault(), cmd->text, ((Vector3){0, 0, 0}), world_font_size, cmd->font_spacing, 1.0f, true, w2rl_color(cmd->font_color));
			}
			else
			{
    			rlTranslatef(cmd->position.x, cmd->position.z, 0.0f);
    			rlRotatef(yaw * RAD2DEG, 0, 0, 1);  // Z rotation in screen space
    			rlTranslatef(-cmd->origin.x, -cmd->origin.z, 0.0f);

				DrawTextEx(GetFontDefault(), cmd->text, ((Vector2){ 0.0f, 0.0f }), cmd->font_size, cmd->font_spacing, w2rl_color(cmd->font_color));
			}

		rlPopMatrix();
	}
}

static inline void wm_raylib_rendering_handle_draw_verts(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world;
	struct w_rendering_cmd_draw_verts *cmd = payload;

	w_mat4 rs = w_mat4_from_trs(
		((w_vec3){0, 0, 0}),  // no position in matrix
		cmd->rotation,
		cmd->scale
	);

	rlPushMatrix();
	rlTranslatef(cmd->position.x, cmd->position.y, cmd->position.z);  // 3. move to world pos
	rlMultMatrixf(rs.m);                                 // 2. rotate + scale
	rlTranslatef(-cmd->origin.x, -cmd->origin.y, -cmd->origin.z);       // 1. shift pivot

	// draw verts
	switch (cmd->draw_mode) {
		case W_RENDERING_DRAW_VERT_MODE_LINES:
			rlBegin(RL_LINES);
			break;
		case W_RENDERING_DRAW_VERT_MODE_TRIANGLES:
			rlBegin(RL_TRIANGLES);
			break;
		case W_RENDERING_DRAW_VERT_MODE_QUADS:
			rlBegin(RL_QUADS);
			break;
		default:
			rlBegin(RL_TRIANGLES);
			break;
	}

	// set vert color
	rlColor4ub(cmd->color.r, cmd->color.g, cmd->color.b, cmd->color.a);

	// draw each vert
	for (size_t i = 0; i < cmd->verts_length; ++i)
	{
		w_vec3 vert = cmd->verts[i];
		rlVertex3f(vert.x, vert.y, vert.z);
	}

	rlEnd();
	rlPopMatrix();
}

#endif /* end of include guard WM_RAYLIB_RENDERING_HANDLERS_DRAW_H */
