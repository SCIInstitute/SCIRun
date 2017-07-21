#ifndef IAUNS_ARC_BALL_H
#define IAUNS_ARC_BALL_H

#include <stdint.h>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace CPM_ARC_BALL_NS {

/// A reimplementation of Ken Shoemake's arcball camera. SCIRun 4's camera
/// system is based off of Ken's code. The Code appears in Graphics Gems 4,
/// III.1.
/// Unless specified otherwise, all calculations and variables stored in this
/// class are relative to the target coordinate system (TCS) for which there is
/// a transformation from screen space to TCS given by the screenToTCS
/// constructor parameter.
/// If the screenToTCS parameter in the constructor is left as the identity
/// matrix then all values are given in screen coordinates.
/// Screen coordinates are (x \in [-1,1]) and (y \in [-1,1]) where (0,0) is the
/// center of the screen.
class ArcBall
{
public:
  /// \param center         Center of the arcball in TCS (screen coordinates if
  ///                       screenToTCS = identity). Generally this will
  ///                       always be (0,0,0). But you may move the center
  ///                       in and out of the screen plane to various effect.
  /// \param radius         Radius in TCS. For screen coordinates, a good
  ///                       default is 0.75.
  /// \param screenToTCS    Transformation from screen coordinates
  ///                       to TCS. \p center and \p radius are given in TCS.
  ArcBall(const glm::vec3& center, glm::float_t radius,
          const glm::mat4& screenToTCS = glm::mat4());
  virtual ~ArcBall();

  /// Initiate an arc ball drag given the mouse click in screen coordinates.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void beginDrag(const glm::vec2& mouseScreenCoords);

  /// Informs the arcball when the mouse has been dragged.
  /// \param mouseScreenCoords  Mouse screen coordinates.
  void drag(const glm::vec2& mouseScreenCoords);

  /// Sets the camera to a specific location and up
  void setLocationOnSphere(glm::vec3 location, glm::vec3 up);

  /// Retrieves the current transformation in TCS.
  /// Obtains full transformation of object in question. If the arc ball is
  /// being used to control camera rotation, then this will contain all
  /// concatenated camera transformations. The current state of the camera
  /// is stored in the quaternions mQDown and mQNow. mMatNow is calculated
  /// from mQNow.
  glm::mat4 getTransformation() const;

private:

  /// Calculates our position on the ArcBall from 2D mouse position.
  /// \param tscMouse   TSC coordinates of mouse click.
  glm::vec3 mouseOnSphere(const glm::vec3& tscMouse);

  /// Construct a unit quaternion from two points on the unit sphere.
  static glm::quat quatFromUnitSphere(const glm::vec3& from, const glm::vec3& to);

  glm::vec3     mCenter;        ///< Center of the arcball in target coordinate system.
  glm::float_t  mRadius;        ///< Radius of the arcball in target coordinate system.

  glm::quat     mQNow;          ///< Current state of the rotation taking into account mouse.
                                ///< Essentially QDrag * QDown (QDown is a applied first, just
                                ///< as in matrix multiplication).
  glm::quat     mQDown;         ///< State of the rotation since mouse down.
  glm::quat     mQDrag;         ///< Dragged transform. Knows nothing of any prior
                                ///< transformations.

  glm::vec3     mVNow;          ///< Most current TCS position of mouse (during drag).
  glm::vec3     mVDown;         ///< TCS position of mouse when the drag was begun.
  glm::vec3     mVSphereFrom;   ///< vDown mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.
  glm::vec3     mVSphereTo;     ///< vNow mapped to the sphere of 'mRadius' centered at 'mCenter' in TCS.

  glm::mat4     mMatNow;        ///< Matrix representing the current rotation.

  /// Transform from screen coordinates to the target coordinate system.
  glm::mat4     mScreenToTCS;
};

} // namespace CPM_ARC_BALL_NS

#endif
