/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_camera_systems
 * @created     : Wednesday Apr 22, 2026 16:40:33 CST
 */

#include "modules/rendering/whisker_rendering_camera.h"
#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"
#include "whisker_debug.h"

/******************
*  sync systems  *
******************/
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
