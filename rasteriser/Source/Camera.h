#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <stdlib.h>
#include <vector>

#define CAMERA_MODE_0 0
#define CAMERA_MODE_1 1
#define CAMERA_MODE_2 2
#define DEG2RAD       0.0174533

class Camera
{
public:
  /* Camera position */
  glm::vec4 pos;
  /* Camera-to-world matrix */
  glm::mat4 c2w;
  /* Focal length */
  float f;
  /* Color mode */
  int colorMode;
  /* Display orthographic view */
  bool orthView;
  /**/
  int hueAdjustment;

  Camera(glm::vec4 pos, float f, int colorMode, glm::mat4 c2w)
  : c2w(c2w), f(f), colorMode(colorMode)
  {
    SetCameraPos(pos);
    orthView = false;
    hueAdjustment = 0;
  }

  void SetCameraPos(glm::vec4 pos) {

    float xDot = pos.x * c2w[0][0] +
                   pos.y * c2w[1][0] +
                   pos.z * c2w[2][0];

    float yDot = pos.x * c2w[0][1] +
                   pos.y * c2w[1][1] +
                   pos.z * c2w[2][1];

    float zDot = pos.x * c2w[0][2] +
                   pos.y * c2w[1][2] +
                   pos.z * c2w[2][2];

    c2w[0][3] += -xDot;
    c2w[1][3] += -yDot;
    c2w[2][3] += -zDot;

  }

  glm::vec4 GetCameraPos() {
    /* Create the result vector */
    glm::vec4 pos = glm::vec4(
      c2w[0][3],
      c2w[1][3],
      c2w[2][3],
      c2w[3][3]);
    return pos;
  }

  void SetCameraRot(glm::vec3 rot) {
    /* Degrees to radians rotation */
    glm::vec3 rpf = (float)DEG2RAD * rot;
    /* Create and apply rotations */
    if(rot.z > 0 || rot.z < 0) {
      float sin, cos;
      sincosf(rpf.z, &sin, &cos);
      glm::mat4 rotZ = glm::mat4(
        cos,-sin, 0  , 0,
        sin, cos, 0  , 0,
        0  , 0  , 1  , 0,
        0  , 0  , 0  , 1
        );
      c2w = rotZ * c2w;
    }
    if(rot.y > 0 || rot.y < 0) {
      float sin, cos;
      sincosf(rpf.y, &sin, &cos);
      glm::mat4 rotY = glm::mat4(
        cos, 0  , sin, 0,
        0  , 1  , 0  , 0,
       -sin, 0  , cos, 0,
        0  , 0  , 0  , 1
        );
      c2w = rotY * c2w;
    }
    if(rot.x > 0 || rot.x < 0) {
      float sin, cos;
      sincosf(rpf.x, &sin, &cos);
      glm::mat4 rotX = glm::mat4(
        1  , 0  , 0  , 0,
        0  , cos,-sin, 0,
        0  , sin, cos, 0,
        0  , 0  , 0  , 1
        );
      c2w = rotX * c2w;
    }
  }

};

#endif
