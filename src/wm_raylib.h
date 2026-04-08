/**
 * @author      : ElGatoPanzon
 * @file        : wm_raylib
 * @created     : Wednesday Apr 01, 2026 13:08:15 CST
 * @description : Raylib module for whisker
 */

#ifndef WM_RAYLIB_H
#define WM_RAYLIB_H

#include "whisker.h"
#include "wm_raylib_macros.h"
#include "wm_raylib_rendering.h"

#include <stdbool.h>

// module configuration
struct wm_raylib_module_config {
    bool rendering;  // Enable rendering subsystem
    bool input;      // Enable input subsystem (future)
    bool audio;      // Enable audio subsystem (future)
};

// initialize the whisker-module-raylib module
void wm_raylib_init(struct w_ecs_world *world, const struct wm_raylib_module_config *config);

// cleanup the whisker-module-raylib module
void wm_raylib_free(struct w_ecs_world *world);

#endif /* WM_RAYLIB_H */
