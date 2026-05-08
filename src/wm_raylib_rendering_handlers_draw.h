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


static void DrawTextCodepointUniversial(Font font, int codepoint, Vector3 position, float fontSize, bool backface, Color tint, Vector2 shadowOffset, Color shadowColor)
{
    int index = GetGlyphIndex(font, codepoint);
    float scale = fontSize/(float)font.baseSize;

    Rectangle srcRec = { font.recs[index].x - (float)font.glyphPadding, font.recs[index].y - (float)font.glyphPadding,
                         font.recs[index].width + 2.0f*font.glyphPadding, font.recs[index].height + 2.0f*font.glyphPadding };

    bool is3D = glIsEnabled(GL_DEPTH_TEST);
    bool hasShadow = (shadowOffset.x != 0.0f || shadowOffset.y != 0.0f);

    float width, height;
    if (is3D)
    {
        position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)/(float)font.baseSize*scale;
        position.z += (float)(font.glyphs[index].offsetY - font.glyphPadding)/(float)font.baseSize*scale;
        width  = (float)(font.recs[index].width  + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
        height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)/(float)font.baseSize*scale;
    }
    else
    {
        position.x += (float)(font.glyphs[index].offsetX - font.glyphPadding)*scale;
        position.y += (float)(font.glyphs[index].offsetY - font.glyphPadding)*scale;
        width  = (float)(font.recs[index].width  + 2.0f*font.glyphPadding)*scale;
        height = (float)(font.recs[index].height + 2.0f*font.glyphPadding)*scale;
    }

    if (font.texture.id > 0)
    {
        const float x = 0.0f;
        const float y = 0.0f;
        const float z = 0.0f;

        const float tx = srcRec.x/font.texture.width;
        const float ty = srcRec.y/font.texture.height;
        const float tw = (srcRec.x+srcRec.width)/font.texture.width;
        const float th = (srcRec.y+srcRec.height)/font.texture.height;

        rlCheckRenderBatchLimit((4 + 4*(is3D && backface)) * (1 + hasShadow));
        rlSetTexture(font.texture.id);

        // shadow pass: same glyph at offset position, drawn behind
        if (hasShadow)
        {
            rlPushMatrix();
                // 3D: scale offset from pixels to world units (same as glyph offsets)
                // 2D: use offset directly in pixel space
                if (is3D)
                    rlTranslatef(position.x + shadowOffset.x/2.0f/(float)font.baseSize*scale, position.y - 0.001f, position.z + shadowOffset.y/2.0f/(float)font.baseSize*scale);
                else
                    rlTranslatef(position.x + shadowOffset.x, position.y + shadowOffset.y, position.z - 0.001f);

                rlBegin(RL_QUADS);
                    rlColor4ub(shadowColor.r, shadowColor.g, shadowColor.b, shadowColor.a);

                    if (is3D)
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
        }

        // normal text pass
        rlPushMatrix();
            rlTranslatef(position.x, position.y, position.z);

            rlBegin(RL_QUADS);
                rlColor4ub(tint.r, tint.g, tint.b, tint.a);

                if (is3D)
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

/* Draw text using unified 2D/3D path. Mode is detected once via GL_DEPTH_TEST
   and drives coordinate layout and advance formula:
   - 3D: XZ plane, world-unit advances (divided by baseSize)
   - 2D: XY plane, pixel advances (scaled by fontSize/baseSize)
   lineHeight is a multiplier for vertical spacing between lines (1.0 = default) */
static void DrawTextUnified(Font font, const char *text, Vector3 position, float fontSize, float spacing, float lineHeight, bool backface, Color tint, Vector2 shadowOffset, Color shadowColor)
{
    int length = TextLength(text);
    float textOffsetY = 0.0f;
    float textOffsetX = 0.0f;
    float scale = fontSize/(float)font.baseSize;
    if (lineHeight == 0) lineHeight = 1.0f;
    bool is3D = glIsEnabled(GL_DEPTH_TEST);

    for (int i = 0; i < length;)
    {
        int codepointByteCount = 0;
        int codepoint = GetCodepoint(&text[i], &codepointByteCount);
        int index = GetGlyphIndex(font, codepoint);

        // NOTE: Normally we exit the decoding sequence as soon as a bad byte is found (and return 0x3f)
        // but we need to draw all of the bad bytes using the '?' symbol moving one byte
        if (codepoint == 0x3f) codepointByteCount = 1;

        if (codepoint == '\n')
        {
            textOffsetY += (is3D ? scale + 1.0f/(float)font.baseSize*scale : fontSize + 2) * lineHeight;
            textOffsetX = 0.0f;
        }
        else
        {
            if ((codepoint != ' ') && (codepoint != '\t'))
            {
                Vector3 pos = is3D
                    ? (Vector3){ position.x + textOffsetX, position.y,             position.z + textOffsetY }
                    : (Vector3){ position.x + textOffsetX, position.y + textOffsetY, 0.0f };
                DrawTextCodepointUniversial(font, codepoint, pos, fontSize, backface, tint, shadowOffset, shadowColor);
            }

            if (font.glyphs[index].advanceX == 0)
                textOffsetX += is3D ? (float)(font.recs[index].width  + spacing)/(float)font.baseSize*scale
                                    : (float) font.recs[index].width  *scale + spacing;
            else
                textOffsetX += is3D ? (float)(font.glyphs[index].advanceX + spacing)/(float)font.baseSize*scale
                                    : (float) font.glyphs[index].advanceX  *scale + spacing;
        }

        i += codepointByteCount;
    }
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
		rlPushMatrix();

		bool is3D = glIsEnabled(GL_DEPTH_TEST);

		// if not 3D, re-write for raylibs odd screen mode camera
		// note: this is mostly to keep the incoming XZ and -Z convention for
		// both world (camera) and overlay (screen) phases
		if (!is3D)
		{
			// rotate +90 degrees on X to negate the incoming -90 degrees on X
			cmd->rotation = w_quat_mul(cmd->rotation, w_quat_rotation_x_deg(90.0f));

			// swap Z and Y for position and scale and origin
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

		// build matrix without position
		w_mat4 rs = w_mat4_from_trs(
			((w_vec3){0, 0, 0}),
			cmd->rotation,
			cmd->scale
		);

		rlTranslatef(cmd->position.x, cmd->position.y, cmd->position.z);
		rlMultMatrixf(rs.m);
		rlTranslatef(-cmd->origin.x, -cmd->origin.y, -cmd->origin.z);

		// set font size to world pixel scale
		if (is3D)
		{
			float base_size = (float)GetFontDefault().baseSize;
			cmd->font_size = ((float)cmd->font_size * base_size) / W_RENDERING_WORLD_PIXEL_SCALE;
		}

		// draw the text
		DrawTextUnified(GetFontDefault(), cmd->text, ((Vector3){ 0.0f, 0.0f, 0.0f }), cmd->font_size, cmd->font_spacing, cmd->font_line_height, true, w2rl_color(cmd->font_color), w2rl_vec2(cmd->font_shadow), w2rl_color(cmd->font_shadow_color));

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
