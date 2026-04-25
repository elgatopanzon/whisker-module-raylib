/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering
 * @created     : Thursday Apr 02, 2026 14:47:00 CST
 * @description : Raylib rendering subsystem
 */

#ifndef WM_RAYLIB_RENDERING_H
#define WM_RAYLIB_RENDERING_H

#include "whisker.h"
#include "modules/whisker_module_ids.h"
#include "modules/rendering/whisker_rendering.h"
#include "modules/scheduler_defaults/whisker_scheduler_defaults.h"
#include <raylib.h>

#define WM_MODULE_RESOURCE_RAYLIB_RENDERING_ID 10000
#define WM_RAYLIB_RENDER_STATE_MODULE_ID WM_MODULE_RESOURCE_ID(RAYLIB_RENDERING, 0)
#define wm_raylib_rendering_get_render_state(world) w_ecs_get_module_resource(world, WM_RAYLIB_RENDER_STATE_MODULE_ID);

// the raylib render state
struct wm_raylib_render_state
{
	// everything we draw will be written to the render texture
    RenderTexture2D render_texture;

    // there is only 1 active raylib camera
    // activated camera entities write their camera components into this
    Camera3D render_camera;
};

// initialize the raylib rendering module
void wm_raylib_rendering_init(struct w_ecs_world *world);

// cleanup the raylib rendering module
void wm_raylib_rendering_free(struct w_ecs_world *world);


/*************
*  helpers  *
*************/

uint32_t wm_raylib_rendering_build_window_config_flags(struct w_rendering_display_config *display_config, struct w_rendering_render_config *render_config);


#endif /* end of include guard WM_RAYLIB_RENDERING_H */
