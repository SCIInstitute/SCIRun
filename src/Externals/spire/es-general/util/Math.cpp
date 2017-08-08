
#include "Math.hpp"

// Make the 4x4 matrix streamable.
std::ostream& operator<< (std::ostream& os, const glm::mat4& m)
{
  os << " " << m[0][0] << " " << m[1][0] << " " << m[2][0] << " " << m[3][0] << "\n";
  os << " " << m[0][1] << " " << m[1][1] << " " << m[2][1] << " " << m[3][1] << "\n";
  os << " " << m[0][2] << " " << m[1][2] << " " << m[2][2] << " " << m[3][2] << "\n";
  os << " " << m[0][3] << " " << m[1][3] << " " << m[2][3] << " " << m[3][3] << "\n";

  return os;
}

std::ostream& operator<< (std::ostream& os, const glm::vec4& v)
{
  os << "(" << v.x << ", " << v.y << ", " << v.z << ", " << v.w << ")\n";
  return os;
}


std::ostream& operator<< (std::ostream& os, const glm::vec3& v)
{
  os << "(" << v.x << ", " << v.y << ", " << v.z << ")\n";
  return os;
}

std::ostream& operator<< (std::ostream& os, const glm::vec2& v)
{
  os << "(" << v.x << ", " << v.y << ")\n";
  return os;
}

std::ostream& operator<< (std::ostream& os, const glm::quat& q)
{
  os << "(" << q.w << ", " << q.x << ", " << q.y << "," << q.z << ")\n";
  return os;
}




