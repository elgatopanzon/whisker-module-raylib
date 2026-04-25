/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering
 * @created     : Thursday Apr 02, 2026 15:12:36 CST
 * @description : Raylib rendering subsystem
 */

#include "wm_raylib_rendering.h"
#include "wm_raylib_rendering_systems.c"
#include "wm_raylib_rendering_camera_systems.c"

void wm_raylib_rendering_init(struct w_ecs_world *world)
{
	// alloc raylib rendering state resource
	struct wm_raylib_render_state *render_state = w_mem_xcalloc_t(1, struct wm_raylib_render_state);
	w_ecs_set_module_resource(world, WM_MODULE_RESOURCE_RAYLIB_RENDERING_ID, render_state);

	// register systems
	wm_raylib_rendering_init_window_register(world);
	wm_raylib_rendering_init_render_texture_register(world);
	wm_raylib_rendering_render_state_sync_register(world);
	wm_raylib_rendering_activate_render_texture_register(world);
	wm_raylib_rendering_deactivate_render_texture_register(world);
	wm_raylib_rendering_clear_window_background_register(world);
	wm_raylib_rendering_draw_render_texture_register(world);
	wm_raylib_rendering_handle_window_close_register(world);
	wm_raylib_rendering_filter_apply_register(world);

	// camera
	wm_raylib_rendering_camera_state_sync_register(world);
}

void wm_raylib_rendering_free(struct w_ecs_world *world)
{
	free(w_ecs_get_module_resource(world, WM_MODULE_RESOURCE_RAYLIB_RENDERING_ID));
	w_ecs_clear_module_resource(world, WM_MODULE_RESOURCE_RAYLIB_RENDERING_ID);
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
