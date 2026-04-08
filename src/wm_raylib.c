/**
 * @author      : ElGatoPanzon
 * @file        : wm_raylib
 * @created     : Wednesday Apr 01, 2026 13:08:15 CST
 * @description : Raylib module for whisker
 */

#include "wm_raylib.h"

void wm_raylib_init(struct w_ecs_world *world, const struct wm_raylib_module_config *config)
{
	if (config && config->rendering) {
		wm_raylib_rendering_init(world);
	}
}

void wm_raylib_free(struct w_ecs_world *world)
{
	wm_raylib_rendering_free(world);
}
