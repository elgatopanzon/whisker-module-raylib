/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_handlers_camera
 * @created     : Sunday May 03, 2026 20:13:09 CST
 * @description : Render command handlers - camera begin/end 3D
 */

#ifndef WM_RAYLIB_RENDERING_HANDLERS_CAMERA_H
#define WM_RAYLIB_RENDERING_HANDLERS_CAMERA_H

#include "whisker_debug.h"
#include "modules/rendering/whisker_rendering_camera.h"

#include "wm_raylib_rendering.h"
#include "wm_raylib_macros.h"

#include "rlgl.h"

#include "modules/rendering/whisker_rendering.h"
#include "modules/rendering/whisker_rendering_commands.h"

static inline void wm_raylib_rendering_handle_camera_begin_3d(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world;
	struct w_rendering_cmd_camera_begin_3d *cmd = payload;

	// draw any outstanding render batches
	rlDrawRenderBatchActive();

	// set to projection mode and upload matrix
	rlMatrixMode(RL_PROJECTION);
	rlPushMatrix();
	rlLoadIdentity();

	switch (cmd->projection_type) {
		case W_RENDERING_CAMERA_PROJECTION_PERSPECTIVE:
			rlFrustum(-cmd->right, cmd->right, -cmd->top, cmd->top, cmd->near_clip, cmd->far_clip);
			break;
		case W_RENDERING_CAMERA_PROJECTION_ORTHOGRAPHIC:
			rlOrtho(-cmd->right, cmd->right, -cmd->top, cmd->top, cmd->near_clip, cmd->far_clip);
			break;
		default:
			break;
	}

	// switch matrix mode to model view to prepare for rendering models
	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();

	// calculate the view matrix from the camera
	rlMultMatrixf(cmd->view_matrix.m);

	// enable depth testing (respect Z)
	rlEnableDepthTest();
}

static inline void wm_raylib_rendering_handle_camera_end_3d(
	struct w_ecs_world *world,
	void *payload
)
{
	(void)world; (void)payload;

	// draw pending render batches
	rlDrawRenderBatchActive();

	// pop the previous active camera matrix and reset to modelview mode
	rlMatrixMode(RL_PROJECTION);
	rlPopMatrix();

	rlMatrixMode(RL_MODELVIEW);
	rlLoadIdentity();

	// disable depth testing (ignore Z)
	rlDisableDepthTest();
}

#endif /* end of include guard WM_RAYLIB_RENDERING_HANDLERS_CAMERA_H */
