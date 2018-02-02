#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <iostream>
#include <stdint.h>
#include <math.h>  //only need abs

#include <glm/glm.hpp>
#include <SDL.h>

#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "Camera.h"

using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;

////////////////////////////////////////////////////////////////////////////////
//
// Data structures.
//
////////////////////////////////////////////////////////////////////////////////

struct Intersection {
  vec4 position;
  float distance;
  int triangleIndex;
};

////////////////////////////////////////////////////////////////////////////////
//
// Function signatures.
//
////////////////////////////////////////////////////////////////////////////////

void Update();

void Draw(screen* screen);

void DrawRoom(
  screen* screen,
  Camera &camera,
  std::vector<Triangle>& triangles,
  int focalLength);

bool ClosestIntersection(
  vec4 start,
  vec4 dir,
  const std::vector<Triangle>& triangles,
  Intersection& closestIntersection);

void buildCameraRay(
  int i,
  int j,
  vec4& start,
  vec4& dir);

float distance(vec4& i, vec4& j);

#endif
