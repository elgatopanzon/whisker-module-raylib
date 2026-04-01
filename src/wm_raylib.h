/**
 * @author      : ElGatoPanzon
 * @file        : wm_raylib
 * @created     : Wednesday Apr 01, 2026 13:08:15 CST
 * @description : Raylib module for whisker
 */

#ifndef WM_RAYLIB_H
#define WM_RAYLIB_H

#include "whisker.h"

// initialize the whisker-module-raylib module
void wm_raylib_init(struct w_ecs_world *world);

// cleanup the whisker-module-raylib module
void wm_raylib_free(struct w_ecs_world *world);

#endif /* WM_RAYLIB_H */
