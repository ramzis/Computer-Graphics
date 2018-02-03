#include "Raytracer.h"

//using namespace std;
using std::vector;

#define SCREEN_WIDTH 200
#define SCREEN_HEIGHT 200
#define FULLSCREEN_MODE false

////////////////////////////////////////////////////////////////////////////////
//
// The main function initializes buffers and starts the engine loop.
//
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {

  /* Screen buffer init */
  screen* screen = InitializeSDL(SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE);

  /* Camera init */
  // TODO move this inside the engine loop or similar.
	//AK: you don't want to reinitialise in the loop repeatedly? it'll slow down the program?
  Camera camera = Camera(
    vec4(0, 0, -1, 1),
    screen->height/2.0);

  /* light source init */
  LightSource light = LightSource( vec4(0, -0.5, -0.7, 1.0), 14.f*vec3(1,1,1));

  //TODO: create world obj? to hold light source, camera, triangles etc

  /* Model data init */
  vector<Triangle> triangles;
  LoadTestModel(triangles);

  /* The engine loop */
  while (NoQuitMessageSDL()) {

    /* Initializes variables */
    // TODO Start();

    /* Updates variables every frame */
    Update(camera);

    /* Clears the screen buffer */
    memset(screen->buffer, 0, screen->height * screen->width * sizeof(uint32_t));

    /* Updates the screen buffer a.k.a renders */
    DrawRoom(screen, camera, triangles, light);

    /* Sends the screen buffer for drawing */
    SDL_Renderframe(screen);
  }

  SDL_SaveImage(screen, "screenshot.bmp");

  KillSDL(screen);

  return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Updates variable values every frame.
//
////////////////////////////////////////////////////////////////////////////////
void Update(Camera &camera) {
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2 - t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
  const Uint8* keystate = SDL_GetKeyboardState(0);
  float cameraSpeed = 0.01;
  if(keystate[SDL_SCANCODE_UP]) {
    camera.pos.z += cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_DOWN]) {
    camera.pos.z -= cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_LEFT]) {
    camera.pos.x += cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_RIGHT]) {
    camera.pos.x -= cameraSpeed;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Draws the frame by raycasting against objects in the scene.
//
////////////////////////////////////////////////////////////////////////////////
// TODO Rename to Raycast or Render etc., and remake logic accordingly.
// Should be generalised for all rendering purposes.

void DrawRoom(
  screen* screen,
  Camera &camera,
  std::vector<Triangle>& triangles,
	LightSource lightSource) {

  bool intersects = false;

  Intersection intersection;

  vec3 colour;
  vec4 d;

  for (int i = 0; i < screen->width; i++) {
    for (int j = 0; j < screen->height; j++) {

      /* Computes ray direction */
      d = vec4(i - screen->width / 2, j - screen->width / 2, camera.f, 1);

      /* Checks if ray intersects */
      intersects = ClosestIntersection(camera.pos,
                                       d,
                                       triangles,
                                       intersection);

      /* If intersection occurs, draw a pixel */
      if (intersects) {
        //colour = triangles[intersection.triangleIndex].color;
				colour = DirectLight(intersection, lightSource, triangles);//shouldn't need to pass the entire triangles vector. TODO: make it so we only need to pass something smaller
        PutPixelSDL(screen, i, j, colour);
      }

    }
  }
}


//@param start: it's the starting poitn of the ray vector
//@param dir: the direction the ray vector is travelling
bool ClosestIntersection(  // v0+ue1+ve2=s+td
    vec4 start, vec4 dir, const vector<Triangle>& triangles,
    Intersection& closestIntersection) {

  bool intersects = false;

  closestIntersection.position = vec4(1.1, 1.1, 1.1, 1);
  closestIntersection.distance = 100.0;
  for (uint i = 0; i < triangles.size(); i++) {
    Triangle triangle = triangles[i];
    // get points of triangle
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;

    // gen the plane this triangle is in and vector from start to triangle

    // basis vectors of the triangle
    vec3 e1 = vec3(v1.x - v0.x, v1.y - v0.y, v1.z - v0.z);
    vec3 e2 = vec3(v2.x - v0.x, v2.y - v0.y, v2.z - v0.z);
    // vector from start to corner of triangle
    vec3 b = vec3(start.x - v0.x, start.y - v0.y, start.z - v0.z);

    mat3 A(-(vec3)dir, e1, e2);
    vec3 x = glm::inverse(A) * b;  // x is the intersection point of the plane and ray

    // check intersection is inside triangle boundaries
    bool seven = x.y >= 0;
    bool eight = x.z >= 0;
    bool nine = x.y + x.z <= 1;
    bool eleven = x.x >= 0;

    if (seven && eight && nine && eleven) {

      intersects = true;

      if (x.x < closestIntersection.distance) {
        closestIntersection.position = vec4(v0.x, v0.y, v0.z, 1);//should this be x.x*dir?
        closestIntersection.distance = x.x;
        closestIntersection.triangleIndex = i;
      }
    }
  }
  return intersects;
}


vec3 DirectLight (const Intersection& i, LightSource source, const std::vector<Triangle>& triangles){//TODO: can we figure out how to do this function without the triangle vector?

	vec4 r = vec4(source.pos - i.position);
	float rmag = glm::length(r);
	vec4 rHat = 	r/rmag;//unit vector. direction from source to intersection
	float pi = 3.1415926535898;

	double dotP = dot(rHat, triangles[i.triangleIndex].normal);
//	printf("%f\n", dotP);
	vec3 D = source.color * (float)std::max(dotP, 0.0)/(4*pi*rmag*rmag);
	return D;
}
	
