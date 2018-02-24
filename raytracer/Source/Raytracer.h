#ifndef _RAYTRACER_H
#define _RAYTRACER_H

#include <iostream>
#include <stdint.h>
//#include <math.h>  //only need abs
#include <stdlib.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/ext.hpp>
#include <SDL.h>

#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "Camera.h"
#include "LightSource.h"
#include "omp.h"

using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::dot;

using std::vector;

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

void Update(Camera &camera, LightSource &lightSource);

void UpdateCamera(Camera &camera, const Uint8* keystate, float deltaTime);

void Draw(screen* screen);

void DrawRoom(
  screen* screen,
  Camera &camera,
  std::vector<Triangle> &triangles,
	LightSource &lightSource);

bool ClosestIntersection(
  vec4 start,
  vec4 dir,
  const std::vector<Triangle> &triangles,
  Intersection& closestIntersection,
  bool isShadowRay);

void buildCameraRay(
  int i,
  int j,
  vec4& start,
  vec4& dir);

vec3 DirectLight(
  const Intersection &i,
  LightSource &source,
  const std::vector<Triangle> &triangles);

#endif
