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
