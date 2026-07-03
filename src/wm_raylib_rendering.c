/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering
 * @created     : Thursday Apr 02, 2026 15:12:36 CST
 * @description : Raylib rendering subsystem
 */

#include "wm_raylib_rendering.h"
#include "wm_raylib_rendering_systems.h"

#include "wm_raylib_rendering_handlers_core.h"
#include "wm_raylib_rendering_handlers_framebuffer.h"
#include "wm_raylib_rendering_handlers_camera.h"
#include "wm_raylib_rendering_handlers_draw.h"
#include "wm_raylib_rendering_handlers_text.h"

void wm_raylib_rendering_init(struct w_ecs_world *world)
{
	// alloc raylib rendering state resource
	struct wm_raylib_render_state *render_state = w_mem_xcalloc_t(1, struct wm_raylib_render_state);
	w_ecs_set_module_resource(world, WM_RAYLIB_RENDER_STATE_MODULE_ID, render_state);

	// register render handlers
	struct w_dispatch_buffer *render_buffer = w_rendering_get_render_dispatch_buffer(world);

	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_INIT_WINDOW, wm_raylib_rendering_handle_init_window);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_CLOSE_WINDOW, wm_raylib_rendering_handle_close_window);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_HANDLE_WINDOW_CLOSE, wm_raylib_rendering_handle_window_close);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_FRAMEBUFFER_MAIN_INIT, wm_raylib_rendering_handle_framebuffer_main_init);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_FRAMEBUFFER_MAIN_ACTIVATE, wm_raylib_rendering_handle_framebuffer_main_activate);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_FRAMEBUFFER_MAIN_DEACTIVATE, wm_raylib_rendering_handle_framebuffer_main_deactivate);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_FRAMEBUFFER_MAIN_SET_FILTER, wm_raylib_rendering_handle_framebuffer_main_set_filter);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_CLEAR_COLOR, wm_raylib_rendering_handle_clear_color);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_RENDER_STATE_SYNC, wm_raylib_rendering_handle_render_state_sync);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_CAMERA_BEGIN_3D, wm_raylib_rendering_handle_camera_begin_3d);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_CAMERA_END_3D, wm_raylib_rendering_handle_camera_end_3d);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_DRAW_BEGIN, wm_raylib_rendering_handle_draw_begin);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_DRAW_END, wm_raylib_rendering_handle_draw_end);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_DRAW_MAIN_FRAMEBUFFER, wm_raylib_rendering_handle_draw_main_framebuffer);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_DRAW_TEXT, wm_raylib_rendering_handle_draw_text);
	w_dispatch_buffer_register_handler(render_buffer, W_RENDERING_CMD_DRAW_VERTS, wm_raylib_rendering_handle_draw_verts);
}

void wm_raylib_rendering_free(struct w_ecs_world *world)
{
	free(w_ecs_get_module_resource(world, WM_RAYLIB_RENDER_STATE_MODULE_ID));
	w_ecs_clear_module_resource(world, WM_RAYLIB_RENDER_STATE_MODULE_ID);
}


/*************
*  helpers  *
*************/

uint32_t wm_raylib_rendering_build_window_config_flags(struct w_rendering_display_config *display_config, struct w_rendering_render_config *render_config)
{
	uint32_t flags = 0;

	// display_config flags
	if (display_config->fullscreen)
	{
		flags |= FLAG_FULLSCREEN_MODE;
	}
	if (display_config->resizable)
	{
		flags |= FLAG_WINDOW_RESIZABLE;
	}
	if (display_config->undecorated)
	{
		flags |= FLAG_WINDOW_UNDECORATED;
	}
	if (display_config->transparent)
	{
		flags |= FLAG_WINDOW_TRANSPARENT;
	}
	if (display_config->hidden)
	{
		flags |= FLAG_WINDOW_HIDDEN;
	}
	if (display_config->minimized)
	{
		flags |= FLAG_WINDOW_MINIMIZED;
	}
	if (display_config->maximised)
	{
		flags |= FLAG_WINDOW_MAXIMIZED;
	}
	if (display_config->always_on_top)
	{
		flags |= FLAG_WINDOW_TOPMOST;
	}
	if (display_config->continue_running_minimised)
	{
		flags |= FLAG_WINDOW_ALWAYS_RUN;
	}
	if (display_config->high_dpi)
	{
		flags |= FLAG_WINDOW_HIGHDPI;
	}
	if (display_config->borderless)
	{
		flags |= FLAG_BORDERLESS_WINDOWED_MODE;
	}
	if (display_config->mouse_passthrough)
	{
		flags |= FLAG_WINDOW_MOUSE_PASSTHROUGH;
	}
	if (display_config->vsync)
	{
		flags |= FLAG_VSYNC_HINT;
	}
	// render_config flags
	// TODO: for now, only 4x supported?
	if (render_config->msaa_samples > 0)
	{
		flags |= FLAG_MSAA_4X_HINT;
	}
	if (render_config->interlaced_mode)
	{
		flags |= FLAG_INTERLACED_HINT;
	}

	return flags;
}
