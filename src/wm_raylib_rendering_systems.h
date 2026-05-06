/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_systems
 * @created     : Monday Apr 06, 2026 14:09:30 CST
 */

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

#include "wm_raylib_rendering_handlers_core.h"
#include "wm_raylib_rendering_handlers_framebuffer.h"
#include "wm_raylib_rendering_handlers_camera.h"
#include "wm_raylib_rendering_handlers_draw.h"

w_ecs_simple_system(
	wm_raylib_rendering_flush_render_buffer_commands,
	WM_PHASE_FINAL_RENDER,
{
	// render dispatch buffer
	struct w_dispatch_buffer *render_buffer = w_rendering_get_render_dispatch_buffer(world);
	w_dispatch_buffer_sort(render_buffer, w_dispatch_buffer_compare_by_priority);

	void *render_cmd_payload_ptr;
	struct w_dispatch_entry *dispatch_entry;
	while ((dispatch_entry = w_dispatch_buffer_pop(render_buffer, &render_cmd_payload_ptr)))
	{
		enum W_RENDERING_CMD render_cmd = dispatch_entry->type_id;

		switch (render_cmd) {
			default:
				debug_log(WARN,	raylib_rendering, "unimplemented render buffer command: %d", render_cmd);		
				break;

			/*********************
			*  window commands  *
			*********************/
			case W_RENDERING_CMD_INIT_WINDOW:
				wm_raylib_rendering_handle_init_window(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_CLOSE_WINDOW:
				wm_raylib_rendering_handle_close_window(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_HANDLE_WINDOW_CLOSE:
				wm_raylib_rendering_handle_window_close(world, render_cmd_payload_ptr);
				break;

			/*******************************
			*  main framebuffer commands  *
			*******************************/
			case W_RENDERING_CMD_FRAMEBUFFER_MAIN_INIT:
				wm_raylib_rendering_handle_framebuffer_main_init(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_FRAMEBUFFER_MAIN_ACTIVATE:
				wm_raylib_rendering_handle_framebuffer_main_activate(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_FRAMEBUFFER_MAIN_DEACTIVATE:
				wm_raylib_rendering_handle_framebuffer_main_deactivate(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_FRAMEBUFFER_MAIN_SET_FILTER:
				wm_raylib_rendering_handle_framebuffer_main_set_filter(world, render_cmd_payload_ptr);
				break;

			/*******************
			*  core commands  *
			*******************/
			case W_RENDERING_CMD_CLEAR_COLOR:
				wm_raylib_rendering_handle_clear_color(world, render_cmd_payload_ptr);
				break;

			/*******************
			*  sync commands  *
			*******************/
			case W_RENDERING_CMD_RENDER_STATE_SYNC:
				wm_raylib_rendering_handle_render_state_sync(world, render_cmd_payload_ptr);
				break;

			/********************
			*  camera commands  *
			********************/
			case W_RENDERING_CMD_CAMERA_BEGIN_3D:
				wm_raylib_rendering_handle_camera_begin_3d(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_CAMERA_END_3D:
				wm_raylib_rendering_handle_camera_end_3d(world, render_cmd_payload_ptr);
				break;

			/*******************
			*  draw commands  *
			*******************/
			case W_RENDERING_CMD_DRAW_BEGIN:
				wm_raylib_rendering_handle_draw_begin(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_DRAW_END:
				wm_raylib_rendering_handle_draw_end(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_DRAW_MAIN_FRAMEBUFFER:
				wm_raylib_rendering_handle_draw_main_framebuffer(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_DRAW_TEXT:
				wm_raylib_rendering_handle_draw_text(world, render_cmd_payload_ptr);
				break;
			case W_RENDERING_CMD_DRAW_VERTS:
				wm_raylib_rendering_handle_draw_verts(world, render_cmd_payload_ptr);
				break;
		}
	}
});
