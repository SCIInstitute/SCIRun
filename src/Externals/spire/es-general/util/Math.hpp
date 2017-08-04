#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>           // glm::value_ptr
#include <iostream>

// Make the 4x4 matrix streamable.
std::ostream& operator<< (std::ostream& os, const glm::mat4& m);
std::ostream& operator<< (std::ostream& os, const glm::vec4& v);
std::ostream& operator<< (std::ostream& os, const glm::vec3& v);
std::ostream& operator<< (std::ostream& os, const glm::vec2& v);
std::ostream& operator<< (std::ostream& os, const glm::quat& q);

#define eieo(t)		( 0.5f * (1 - cosf( (t) * glm::pi<float>() )) )
#define fieo(t)		( sinf( (t) * (glm::pi<float>() / 2.0f ) )
#define eifo(t)		( 1.0f - sinf( (1.0f - (t)) * (glm::pi<float>() / 2.0f) ) )

namespace util {

} // namespace util

