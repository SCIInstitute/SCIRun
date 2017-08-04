
#include "GLPlatform.hpp"
#include <stdexcept>

namespace spire {

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

} // namespace spire

