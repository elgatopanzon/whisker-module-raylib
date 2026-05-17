/**
 * @author      : ElGatoPanzon (contact@elgatopanzon.io)
 * @file        : wm_raylib_rendering_rlgl
 * @created     : Sunday May 17, 2026 13:09:47 CST
 * @description : Cross-platform GL wrapper for rlgl.h inclusion
 */

// mostly borrowed from raylib as a way to include GL in other modules for
// simpler GL dispatch calls

// default to OpenGL 3.3 if no graphics API defined
#if !defined(GRAPHICS_API_OPENGL_11) && \
    !defined(GRAPHICS_API_OPENGL_21) && \
    !defined(GRAPHICS_API_OPENGL_33) && \
    !defined(GRAPHICS_API_OPENGL_43) && \
    !defined(GRAPHICS_API_OPENGL_ES2) && \
    !defined(GRAPHICS_API_OPENGL_ES3)
    #define GRAPHICS_API_OPENGL_33
#endif
// openGL 2.1 uses most of OpenGL 3.3 Core functionality
#if defined(GRAPHICS_API_OPENGL_21)
    #define GRAPHICS_API_OPENGL_33
#endif
// openGL 4.3 uses OpenGL 3.3 Core functionality
#if defined(GRAPHICS_API_OPENGL_43)
    #define GRAPHICS_API_OPENGL_33
#endif
// openGL ES 3.0 uses OpenGL ES 2.0 functionality
#if defined(GRAPHICS_API_OPENGL_ES3)
    #define GRAPHICS_API_OPENGL_ES2
#endif

// detect platform if not explicitly defined
#if !defined(PLATFORM_DESKTOP_GLFW) && \
    !defined(PLATFORM_DESKTOP_SDL) && \
    !defined(PLATFORM_DESKTOP_RGFW) && \
    !defined(PLATFORM_WEB) && \
    !defined(PLATFORM_DRM) && \
    !defined(PLATFORM_ANDROID)
    #if defined(_WIN32) || defined(__linux__) || defined(__APPLE__)
        #define PLATFORM_DESKTOP_GLFW
    #endif
    #if defined(__EMSCRIPTEN__)
        #undef PLATFORM_DESKTOP_GLFW
        #define PLATFORM_WEB
    #endif
    #if defined(__ANDROID__)
        #undef PLATFORM_DESKTOP_GLFW
        #define PLATFORM_ANDROID
    #endif
#endif

// expose OpenGL functions from glad (shared library builds)
#if defined(BUILD_LIBTYPE_SHARED)
    #define GLAD_API_CALL_EXPORT
    #define GLAD_API_CALL_EXPORT_BUILD
#endif

// OpenGL 1.1 includes
#if defined(GRAPHICS_API_OPENGL_11)
    #if defined(__APPLE__)
        #include <OpenGL/gl.h>
        #include <OpenGL/glext.h>
    #else
        #if !defined(APIENTRY)
            #if defined(_WIN32)
                #define APIENTRY __stdcall
            #else
                #define APIENTRY
            #endif
        #endif
        #if !defined(WINGDIAPI) && defined(_WIN32)
            #define WINGDIAPI __declspec(dllimport)
        #endif
        #include <GL/gl.h>
    #endif
#endif

// OpenGL 3.3/4.3 Core uses GLAD
#if defined(GRAPHICS_API_OPENGL_33)
    #if !defined(RL_MALLOC)
        #include <stdlib.h>
        #define RL_MALLOC(sz)     malloc(sz)
        #define RL_FREE(p)        free(p)
    #endif
    #define GLAD_MALLOC RL_MALLOC
    #define GLAD_FREE RL_FREE
#endif

// OpenGL ES 3.0 includes
#if defined(GRAPHICS_API_OPENGL_ES3)
    #include <GLES3/gl3.h>
    #define GL_GLEXT_PROTOTYPES
    #include <GLES2/gl2ext.h>
// OpenGL ES 2.0 includes
#elif defined(GRAPHICS_API_OPENGL_ES2)
    #if defined(PLATFORM_DESKTOP_GLFW) || defined(PLATFORM_DESKTOP_SDL)
        // Uses glad_gles2 via raylib
    #else
        // Native OpenGL ES 2.0 (Android, DRM, Web)
        #define GL_GLEXT_PROTOTYPES
        #include <GLES2/gl2.h>
        #include <GLES2/gl2ext.h>
    #endif
    #if defined(PLATFORM_DRM)
        typedef void (GL_APIENTRYP PFNGLDRAWARRAYSINSTANCEDEXTPROC) (GLenum mode, GLint start, GLsizei count, GLsizei primcount);
        typedef void (GL_APIENTRYP PFNGLDRAWELEMENTSINSTANCEDEXTPROC) (GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei primcount);
        typedef void (GL_APIENTRYP PFNGLVERTEXATTRIBDIVISOREXTPROC) (GLuint index, GLuint divisor);
    #endif
#endif

// GLAD is a desktop OpenGL loader - exclude for WASM/Android which use GLES
#if defined(__EMSCRIPTEN__)
    #include <GLES3/gl3.h>
#elif defined(__ANDROID__)
    #include <GLES2/gl2.h>
#else
    #include <external/glad.h>
#endif
#include <rlgl.h>
