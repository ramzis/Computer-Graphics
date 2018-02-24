#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <vector>

#define CAMERA_MODE_0 0
#define CAMERA_MODE_1 1
#define CAMERA_MODE_2 2

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

  Camera(glm::vec4 pos, float f, int colorMode, glm::mat4 c2w)
  : c2w(c2w), f(f), colorMode(colorMode) 
  {
    SetCameraPos(pos);
  }

  void SetCameraPos(glm::vec4 pos) {
    /* Create a translation matrix */
    glm::mat4 translationMatrix = glm::mat4(
      1,0,0,pos.x,
      0,1,0,pos.y,
      0,0,1,pos.z,
      0,0,0,1);
    /* Add translation to the Camera-to-World matrix */
    c2w = translationMatrix * c2w;
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

};

#endif
