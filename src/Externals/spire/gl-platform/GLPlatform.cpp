
#include "GLPlatform.hpp"
#include <stdexcept>

namespace CPM_GL_PLATFORM_NS {

void glPlatformInit()
{
#ifdef WIN32
  GLenum err = glewInit();
  if (GLEW_OK != err)
  {
    throw std::runtime_error("GLEW failed to initialize.");
  }
#endif
}

} // namespace CPM_GL_PLATFORM_NS

