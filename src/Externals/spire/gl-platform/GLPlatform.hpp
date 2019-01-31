#ifndef SPIRE_GL_PLATFORM_HPP
#define SPIRE_GL_PLATFORM_HPP

#include <es-log/trace-log.h>
#include <spire/scishare.h>

namespace spire {

/// This function is only required on Windows (due to GLEW -- unless you alerady
/// call GLEW init somewhere else) and it only calls glewInit(). It's best to
/// call it on all platforms to maintain consistency. But calling it is *not*
/// mandatory.
SCISHARE void glPlatformInit();

} // namespace spire

#if defined(USE_CORE_PROFILE_3) && defined(USE_CORE_PROFILE_4)
  #error Both USE_CORE_PROFILE_3 and USE_CORE_PROFILE_4 are defined. Ensure that only one is defined.
#endif

// See: http://stackoverflow.com/questions/5919996/how-to-detect-reliably-mac-os-x-ios-linux-windows-in-c-preprocessor
// and, http://sourceforge.net/p/predef/wiki/OperatingSystems/

// Note, for platforms that you can't uniquely detect through preprocessor
// definitions, use build directives.
#if _WIN32
  #define NOMINMAX
  #include <Windows.h>
  #include <GL/glew.h>
  #include <GL/gl.h>
#elif defined EMSCRIPTEN
  #define GL_GLEXT_PROTOTYPES
  #include <GL/glfw.h>
  #include <GL/glext.h>
#elif __APPLE__
  #include "TargetConditionals.h"

  #if TARGET_IPHONE_SIMULATOR
    #define GL_PLATFORM_USING_IOS
  #elif TARGET_OS_IPHONE
    #define GL_PLATFORM_USING_IOS
  #elif TARGET_OS_MAC
    #define GL_PLATFORM_USING_OSX
  #else
    #error Unsupported Apple platform.
  #endif

  #if defined GL_PLATFORM_USING_IOS
    #define USE_OPENGL_ES
    #import <OpenGLES/ES2/gl.h>
    #import <OpenGLES/ES2/glext.h>
  #elif defined GL_PLATFORM_USING_OSX
    #include <OpenGL/gl.h>
    #include <OpenGL/glext.h>
    #if defined(USE_CORE_PROFILE_3) || defined(USE_CORE_PROFILE_4)
      // Currently mac places gl4 specific definitions in the gl3 header. Change
      // when they update this.
      #include <OpenGL/gl3.h>
    #endif
  #else
    #error Apple GLPlatform logic error
  #endif
#elif __ANDROID__
  #define USE_OPENGL_ES
  #include <GLES2/gl2.h>
  #include <GLES2/gl2ext.h>
#elif __linux
  #define GL_GLEXT_PROTOTYPES
  #include <GL/gl.h>
  #include <GL/glext.h>
#elif __unix // all unices not caught above
  #error General unix not supported - try defining GL_PLATFORM_USING_LINUX and disabling this error.
#elif __posix
  #error General posix not supported - try defining GL_PLATFORM_USING_LINUX and disabling this error.
#else
  #error Unknown unsupported platform.
#endif



#ifndef GL_PLATFORM_NO_DEBUGGING
#include <iostream>

// Misc GL debugging.
#define MAX_GL_ERROR_COUNT 10
#ifdef SPIRE_GLDEBUG_DEBUG
# define GL(stmt)                                                      \
  do {                                                                 \
    GLenum glerr;                                                      \
    unsigned int iCounter = 0;                                         \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      std::cerr << "GL error calling" << #stmt << " before line " << __LINE__ << " (" << __FILE__ << "): " << static_cast<unsigned>(glerr) << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
    stmt;                                                              \
    iCounter = 0;                                                      \
    while((glerr = glGetError()) != GL_NO_ERROR) {                     \
      std::cerr << "'" << #stmt << "' on line " << __LINE__ << " (" << __FILE__ << ") caused GL error: " << static_cast<unsigned>(glerr) << std::endl; \
      iCounter++;                                                      \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                        \
    }                                                                  \
  } while(0)

# define GL_CHECK()                                                       \
  do {                                                                    \
    GLenum glerr;                                                         \
    unsigned int iCounter = 0;                                            \
    while((glerr = glGetError()) != GL_NO_ERROR) {                        \
      std::cerr << "GL error before line " << __LINE__ << "("   \
                << __FILE__ << "): " << glerr                             \
                << std::endl;                                             \
      iCounter++;                                                         \
      if (iCounter > MAX_GL_ERROR_COUNT) break;                           \
    }                                                                     \
  } while(0)
#else
# define GL(stmt) do { stmt; } while(0)
# define GL_CHECK()
#endif

#endif // #ifndef GL_PLATFORM_NO_DEBUGGING

#endif
