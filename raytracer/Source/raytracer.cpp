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
  LightSource light = LightSource(vec4(0, -0.5, -0.7, 1.0), 14.f*vec3(1,1,1));//that factor of 50 was originally 14 in notes?

  //TODO: create world obj? to hold light source, camera, triangles etc

  /* Model data init */
  vector<Triangle> triangles;
  LoadTestModel(triangles);

  /* The engine loop */
  while (NoQuitMessageSDL()) {

    /* Initializes variables */
    // TODO Start();

    /* Updates variables every frame */
    Update(camera, light);

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
void Update(Camera &camera, LightSource &light) {
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

  if(keystate[SDL_SCANCODE_W]) {
  	light.pos.z += cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_S]) {
  	light.pos.z -= cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_A]) {
  	light.pos.x -= cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_D]) {
  	light.pos.x += cameraSpeed;
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
        float r = ((double) rand() / (RAND_MAX)) + 1;
        float g = ((double) rand() / (RAND_MAX)) + 1;
        float b = ((double) rand() / (RAND_MAX)) + 1;

        // Funky night camera mode
        //colour = vec3(r,g,b);
        //colour = cross((float)0.3*colour, triangles[intersection.triangleIndex].color);
        
        // Grayscale mode
        //colour = vec3(1,1,1); 
        
        colour = triangles[intersection.triangleIndex].color;
		colour *= DirectLight(intersection, lightSource, triangles);//multiply colour by ammount of direct light reaching that point
        
        PutPixelSDL(screen, i, j, colour);
      }

    }
  }
}


//@param start: it's the starting poitn of the ray vector
//@param dir: the direction the ray vector is travelling
bool ClosestIntersection(  // v0+ue1+ve2=s+td
    vec4 start,
    vec4 dir,
    const vector<Triangle>& triangles,
    Intersection& closestIntersection) {

  bool intersects = false;

  closestIntersection.position = start;//vec4(1.1, 1.1, 1.1, 1);
  closestIntersection.distance = 100.0;

  /* Loop through all triangles in the scene */
  for (uint i = 0; i < triangles.size(); i++) {

    Triangle triangle = triangles[i];

    /* Triangle vertices */
    vec4 v0 = triangle.v0;
    vec4 v1 = triangle.v1;
    vec4 v2 = triangle.v2;

    /* The basis vectors of the triangle plane */
    vec3 e1 = vec3(v1-v0);
    vec3 e2 = vec3(v2-v0);
    /* Camera to triangle plane distance */
    vec3 b  = vec3(start-v0);

    // vector from start to corner of triangle
    mat3 A(-(vec3)dir, e1, e2);
    vec3 x = glm::inverse(A) * b;  // x is the intersection point of the plane and ray

    /* Intersection distance and coordinates */
    float t = x.x;
    float u = x.y;
    float v = x.z;

    /* Check if the intersection is in the triangle plane */
    if(t > 0 && u >= 0 && v >= 0 && u + v <= 1) {

      intersects = true;

      if (t < closestIntersection.distance) {
        //closestIntersection.position = vec4(b.x, b.y, b.z, 1); // This is cool shit. Also invert lightsource.
        closestIntersection.position = start+t*dir; // This is correct
        closestIntersection.distance = t;
        closestIntersection.triangleIndex = i;
      }

    }
  }
  return intersects;
}

/*
@param: i, point where (camera) ray intersects with object
@param: source, light source in the room
@param: triangles, vector containing all the traingles, TODO: replace with only the intersecting triangle rather than them all
@return: D, the power as a colour vector
*/
vec3 DirectLight (const Intersection& i, LightSource source, const std::vector<Triangle>& triangles){

	//vector between intersection point and light source 
	vec4 r = vec4(source.pos - i.position);

	//magnitude
	float rmag = glm::length(r);
	//unit vector. direction from source to intersection
	//vec4 rHat = r/rmag;
  	vec4 rHat = glm::normalize(r);
	//lol
	float pi = 3.1415926535898;

	//dot product of rHat and normal of triangle
	double dotP = dot(rHat, triangles[i.triangleIndex].normal);
	//power per real surface
	vec3 D = source.color * (float)std::max(dotP, 0.0)/(4*pi*rmag*rmag);

	// Shadows
	bool intersects = false;
	// Offset to fix shadow-acne, lol
	float bias = 0.01;
	vec4 start = i.position+triangles[i.triangleIndex].normal*bias;
  	vec4 dir = vec4(source.pos - i.position);
  	Intersection intersection;

	intersects = (ClosestIntersection(
		start,
		dir,
		triangles,
		intersection));

	if(intersects && intersection.distance <= rmag) {
		D = vec3(0,0,0);
	}

	return D;
}
