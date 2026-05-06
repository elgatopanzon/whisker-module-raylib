/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_handlers_core
 * @created     : Sunday May 03, 2026 20:13:09 CST
 * @description : Render command handlers - window lifecycle, sync, clear
 */

#ifndef WM_RAYLIB_RENDERING_HANDLERS_CORE_H
#define WM_RAYLIB_RENDERING_HANDLERS_CORE_H

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

static inline void wm_raylib_rendering_handle_init_window(
	struct w_ecs_world *world,
	void *payload
)
{
	struct w_rendering_display_config *display_config = w_rendering_get_display_config(world);
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct w_rendering_cmd_init_window *cmd = payload;

	SetConfigFlags(wm_raylib_rendering_build_window_config_flags(display_config, render_config));

	debug_log(DEBUG, raylib_rendering, "init window: %dx%d (%s)", cmd->window_resolution.x, cmd->window_resolution.y, cmd->window_title);

	InitWindow(cmd->window_resolution.x, cmd->window_resolution.y, cmd->window_title);
}

static inline void wm_raylib_rendering_handle_close_window(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world;
	struct w_rendering_cmd_init_window *cmd = payload;

	debug_log(DEBUG, raylib_rendering, "closing window window: %dx%d (%s)", cmd->window_resolution.x, cmd->window_resolution.y, cmd->window_title);

	CloseWindow();
}

static inline void wm_raylib_rendering_handle_window_close(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)payload;
	if (WindowShouldClose())
	{
		debug_log(DEBUG, raylib_rendering, "window close detected, shutting down world");
		world->update_result = W_WORLD_UPDATE_RESULT_SHUTDOWN;
	}
}

static inline void wm_raylib_rendering_handle_render_state_sync(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)payload;
	struct w_rendering_render_state *render_state = w_rendering_get_render_state(world);
	render_state->render_fps = GetFPS();
	render_state->render_frame_time = GetFrameTime() * 1000;
	render_state->window_resolution.x = GetScreenWidth();
	render_state->window_resolution.y = GetScreenHeight();
}

static inline void wm_raylib_rendering_handle_clear_color(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world;
	struct w_rendering_cmd_clear_color *cmd = payload;
	ClearBackground(w2rl_color(cmd->clear_color));
}

#endif /* end of include guard WM_RAYLIB_RENDERING_HANDLERS_CORE_H */
