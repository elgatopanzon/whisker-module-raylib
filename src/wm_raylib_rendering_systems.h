/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_systems
 * @created     : Monday Apr 06, 2026 14:09:30 CST
 */

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

/******************
*  init systems  *
******************/
// systems running at startup and only once

// init window system runs once at startup to initialise the 
// raylib window and rendering context
w_ecs_simple_system(wm_raylib_rendering_init_window, WM_PHASE_ON_STARTUP, {
	struct w_rendering_display_config *display_config = w_rendering_get_display_config(world);
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);

	char *window_title = w_string_table_lookup(world->string_table, display_config->window_title_id);

	if (!window_title)
	{
		window_title = "Window Title Undefined";
	}

	// build raylib window flags out of display and render config bools
	SetConfigFlags(wm_raylib_rendering_build_window_config_flags(display_config, render_config));

	// init raylib window
	InitWindow(display_config->window_resolution.x, display_config->window_resolution.y, window_title);
});

// init the render texture, the texture is what gets rendered to the screen
w_ecs_simple_system(wm_raylib_rendering_init_render_texture, WM_PHASE_ON_STARTUP, {
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	// init the render texture
	raylib_render_state->render_texture = LoadRenderTexture(render_config->render_resolution.x, render_config->render_resolution.y);
});


/******************
*  sync systems  *
******************/
// sync systems handle raylib<->ecs data e.g. render state


// init the render texture, the texture is what gets rendered to the screen
w_ecs_simple_system(wm_raylib_rendering_render_state_sync, WM_PHASE_FINAL, {
	struct w_rendering_render_state *render_state = w_rendering_get_render_state(world);

	render_state->render_fps = GetFPS();
	render_state->render_frame_time = GetFrameTime() * 1000;

	render_state->window_resolution.x = GetScreenWidth();
	render_state->window_resolution.y = GetScreenHeight();
});


/********************
*  render texture  *
********************/
// the main draw pipeline draws everything to a single texture then draws that

// these 2 systems activate and deactive the render texture ensuring everything
// in the RENDER phases is rendered to this texture
w_ecs_simple_system(wm_raylib_rendering_activate_render_texture, WM_PHASE_PRE_RENDER, {
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	BeginTextureMode(raylib_render_state->render_texture);
});
w_ecs_simple_system(wm_raylib_rendering_deactivate_render_texture, WM_PHASE_POST_RENDER, {
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	EndTextureMode();
});

// clear background system
w_ecs_simple_system(wm_raylib_rendering_clear_window_background, WM_PHASE_PRE_RENDER, {
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);

	if (render_config->clear_enabled)
	{
		ClearBackground(w2rl_color(render_config->window_clear_color));	
	}
});


/***********************
*  filtering systems  *
***********************/

// apply the filter type to the render texture
w_ecs_simple_system(wm_raylib_rendering_filter_apply, WM_RENDER_PHASE_ON_FILTER, {
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	// apply texture filter from render config
	int raylib_filter;
	switch (render_config->texture_filter) {
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
});


/******************
*  draw systems  *
******************/
// the main draw systems, draws the render texture

// draw the render texture to the screen
w_ecs_simple_system(wm_raylib_rendering_draw_render_texture, WM_RENDER_PHASE_ON_DRAW, {
	struct w_rendering_render_config *render_config = w_rendering_get_render_config(world);
	struct w_rendering_render_state *render_state = w_rendering_get_render_state(world);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	// start main drawing and clear
	BeginDrawing();
	ClearBackground(w2rl_color(render_config->render_clear_color));

	// draw final texture, flipping Y source height
	Rectangle rl_source = w2rl_rect(render_state->render_texture_source);
	rl_source.height = -rl_source.height;
	DrawTexturePro(raylib_render_state->render_texture.texture, rl_source, w2rl_rect(render_state->render_texture_destination), w2rl_vec2(render_state->draw_origin), render_state->draw_rotation, WHITE);

	// end drawing
	EndDrawing();
});


/*********************
*  handler systems  *
*********************/
// handlers watch for specific events/world behaviours and perform actions

// trigger world shutdown result when window closed
w_ecs_simple_system(wm_raylib_rendering_handle_window_close, WM_PHASE_POST, {
	struct w_rendering_display_config *display_config = w_rendering_get_display_config(world);
	if (!display_config->handle_window_close)
		return;

	if (WindowShouldClose())
	{
		world->update_result = W_WORLD_UPDATE_RESULT_SHUTDOWN;
	}
});


/********************
*  camera systems  *
********************/

// to render the camera we need to grab the data from the active camera entity
w_ecs_simple_system(
	wm_raylib_rendering_camera_state_sync,
	WM_RENDER_PHASE_PRE_WORLD,
{
	struct w_rendering_camera_state *camera_state = w_ecs_get_module_resource(world, WM_RENDERING_CAMERA_STATE_RESOURCE_ID);
	struct wm_raylib_render_state *raylib_render_state = wm_raylib_rendering_get_render_state(world);

	// set camera values
	raylib_render_state->render_camera.position = w2rl_vec3(camera_state->camera_position);
	raylib_render_state->render_camera.target = w2rl_vec3(camera_state->camera_target);
	raylib_render_state->render_camera.up = w2rl_vec3(camera_state->camera_up);
	raylib_render_state->render_camera.fovy = camera_state->camera_fov_deg;

	// camera project mode
	switch (camera_state->camera_projection) {
		case W_RENDERING_CAMERA_PROJECTION_ORTHOGRAPHIC:
			raylib_render_state->render_camera.projection = CAMERA_ORTHOGRAPHIC;
			break;
		case W_RENDERING_CAMERA_PROJECTION_PERSPECTIVE:
			raylib_render_state->render_camera.projection = CAMERA_PERSPECTIVE;
			break;
			
	}
});
