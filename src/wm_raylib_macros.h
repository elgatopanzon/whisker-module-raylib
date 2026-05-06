/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_macros
 * @created     : Monday Apr 06, 2026 14:19:47 CST
 */

#ifndef WM_RAYLIB_MACROS_H

#define WM_RAYLIB_MACROS_H


#define w2rl_color(c) (Color){.r = c.r, .g = c.g, .b = c.b, .a = c.a}
#define rl2w_color(c) (w_color){.r = c.r, .g = c.g, .b = c.b, .a = c.a}
#define w2rl_rect(r) (Rectangle){.x = r.x, .y = r.y, .width = r.width, .height = r.height}
#define rl2w_rect(r) (w_rect){.x = r.x, .y = r.y, .width = r.width, .height = r.height}
#define w2rl_vec2(v) (Vector2){.x = v.x, .y = v.y}
#define rl2w_vec2(v) (w_vec2){.x = v.x, .y = v.y}
#define w2rl_vec3(v) (Vector3){.x = v.x, .y = v.y, .z = v.z}
#define rl2w_vec3(v) (w_vec3){.x = v.x, .y = v.y, .z = v.z}

#endif /* end of include guard WM_RAYLIB_MACROS_H */

