/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_handlers_framebuffer
 * @created     : Sunday May 03, 2026 20:13:09 CST
 * @description : Render command handlers - framebuffer init/activate/deactivate/filter
 */

#ifndef WM_RAYLIB_RENDERING_HANDLERS_FRAMEBUFFER_H
#define WM_RAYLIB_RENDERING_HANDLERS_FRAMEBUFFER_H

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

static inline void wm_raylib_rendering_handle_framebuffer_main_init(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)payload;
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);
	debug_log(DEBUG, raylib_rendering, "init main framebuffer");
	raylib_render_state->render_texture = LoadRenderTexture(render_config->render_resolution.x, render_config->render_resolution.y);
}

static inline void wm_raylib_rendering_handle_framebuffer_main_activate(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)payload;
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);
	BeginTextureMode(raylib_render_state->render_texture);
}

static inline void wm_raylib_rendering_handle_framebuffer_main_deactivate(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world; (void)payload;
	EndTextureMode();
}

static inline void wm_raylib_rendering_handle_framebuffer_main_set_filter(
	struct w_ecs_world *world,
	void *payload
)
{
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);
	struct w_rendering_cmd_framebuffer_set_filter *cmd = payload;

	int raylib_filter;
	switch (cmd->filter_type) {
		case W_RENDERING_TEXTURE_FILTER_POINT:
			raylib_filter = TEXTURE_FILTER_POINT;
			break;
		case W_RENDERING_TEXTURE_FILTER_BILINEAR:
			raylib_filter = TEXTURE_FILTER_BILINEAR;
			break;
		case W_RENDERING_TEXTURE_FILTER_TRILINEAR:
			raylib_filter = TEXTURE_FILTER_TRILINEAR;
			break;
		case W_RENDERING_TEXTURE_FILTER_ANISOTROPIC:
			switch (render_config->anisotropic_level) {
				case 4:
					raylib_filter = TEXTURE_FILTER_ANISOTROPIC_4X;
					break;
				case 8:
					raylib_filter = TEXTURE_FILTER_ANISOTROPIC_8X;
					break;
				case 16:
					raylib_filter = TEXTURE_FILTER_ANISOTROPIC_16X;
					break;
				default:
					break;
					raylib_filter = TEXTURE_FILTER_ANISOTROPIC_4X;
			}
			break;
		default:
			raylib_filter = TEXTURE_FILTER_POINT;
			break;
	}
	SetTextureFilter(raylib_render_state->render_texture.texture, raylib_filter);
}

#endif /* end of include guard WM_RAYLIB_RENDERING_HANDLERS_FRAMEBUFFER_H */
