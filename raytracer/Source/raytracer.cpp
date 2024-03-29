#include "Raytracer.h"

#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
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
  /* Position in world coordinates */
  vec4 camPos = vec4(0, 0, 2, 1);
  /* Rotation in angles */
  //vec3 rot = vec3(0,0,0);
  /* Camera to world matrix */
  mat4 c2w = mat4(
    1,0,0,0,
    0,1,0,0,
    0,0,1,0,
    0,0,0,1);
  Camera camera = Camera(
    camPos,
    screen->height/2.0,
    0,
    c2w);

  /* Light source init */
  /* Position in world coordinates */
  vec4 lightPos = vec4(0, 0, -1, 1);
  /* Light colour */
  vec3 lightColour = vec3(1, 1, 1);
  /* Light intensity */
  float lightIntensity = 60.0f;
  LightSource light = LightSource(
    lightPos,
    lightColour,
    lightIntensity);

  // TODO: create world obj? to hold light source, camera, triangles etc
  // Tadas: Yep, exactly

  /* Model data init */
  vector<Triangle> triangles;
  LoadTestModel(triangles);

  /* The engine loop */
  while (NoQuitMessageSDL()) {

    /* Initializes variables */
    // TODO Start(); Move this into the world too.

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
    
  /* RAW VALUES AND COMPUTATION */

  /* Get keyboard input state */
  const Uint8* keystate = SDL_GetKeyboardState(0);

  /* Compute frame time */
  static int t = SDL_GetTicks();
  int t2 = SDL_GetTicks();
  float dt = float(t2 - t);
  t = t2;
  
  /* Good idea to remove this */
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  
  /* UPDATE VARIABLES */

  /* Update camera translation, rotation, color mode */
  UpdateCamera(camera, keystate, dt);

  /* Light movement */
  float lightSpeed = 0.001 * dt;

  if(keystate[SDL_SCANCODE_H]) {
    light.pos.z -= lightSpeed;
  }
  if(keystate[SDL_SCANCODE_J]) {
    light.pos.z += lightSpeed;
  }
  if(keystate[SDL_SCANCODE_K]) {
    light.pos.x -= lightSpeed;
  }
  if(keystate[SDL_SCANCODE_L]) {
    light.pos.x += lightSpeed;
  }
  if(keystate[SDL_SCANCODE_U]) {
    light.pos.y -= lightSpeed;
  }
  if(keystate[SDL_SCANCODE_I]) {
    light.pos.y += lightSpeed;
  }

}

////////////////////////////////////////////////////////////////////////////////
//
// Updates the position, rotation and color mode of the camera.
//
////////////////////////////////////////////////////////////////////////////////
void UpdateCamera(Camera &camera, const Uint8* keystate, float deltaTime) {
  
  /* CAMERA MOVEMENT*/

  /* Movement speed per frame */
  float cameraSpeed = 0.001 * deltaTime;
  /* Init translation vector */
  vec4 cameraTranslate = vec4(0,0,0,0);
  /* Update translation vector with raw input */
  if(keystate[SDL_SCANCODE_W]) {
    cameraTranslate.z = -cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_S]) {
    cameraTranslate.z = cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_LEFT]) {
    cameraTranslate.x = cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_RIGHT]) {
    cameraTranslate.x = -cameraSpeed;
  }
  /* Update position */
  camera.SetCameraPos(cameraTranslate);
  /* Rotation speed per frame */
  float cameraRotSpeed = 0.05f * deltaTime;
  /* Init rotation vector */
  vec3 cameraRotate = vec3(0,0,0);
  /* Update rotation vector with raw input */
  if(keystate[SDL_SCANCODE_A]) {
    if(keystate[SDL_SCANCODE_S])
      cameraRotate.y -= cameraRotSpeed;
    else
      cameraRotate.y += cameraRotSpeed;
  }
  if(keystate[SDL_SCANCODE_D]) {
    if(keystate[SDL_SCANCODE_S])
      cameraRotate.y += cameraRotSpeed;
    else
      cameraRotate.y -= cameraRotSpeed;
  }
  if(keystate[SDL_SCANCODE_UP]) {
    cameraRotate.x = -cameraRotSpeed;
  }
  if(keystate[SDL_SCANCODE_DOWN]) {
    cameraRotate.x = cameraRotSpeed;
  }
  /* Update rotation */
  camera.SetCameraRot(cameraRotate);

  /* CAMERA COLOR MODES */

  /* Update camera color mode from raw input */
  if(keystate[SDL_SCANCODE_1]) {
    camera.colorMode = CAMERA_MODE_0;
  }
  if(keystate[SDL_SCANCODE_2]) {
    camera.colorMode = CAMERA_MODE_1;
  }
  if(keystate[SDL_SCANCODE_3]) {
    camera.colorMode = CAMERA_MODE_2;
  }
  if(keystate[SDL_SCANCODE_4]) {
    camera.colorMode = 4;
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
  std::vector<Triangle> &triangles,
  LightSource &lightSource) {

  bool intersects = false;

  Intersection intersection;

  vec3 colour;
  vec4 dir;

  omp_set_num_threads(4);
  #pragma omp parallel default(none) private(dir, colour, intersects, intersection) shared(screen, camera, triangles, lightSource)
  #pragma omp for
  for (int i = 0; i < screen->width; i++) {
    for (int j = 0; j < screen->height; j++) {

      /* Computes ray direction */
      dir = vec4(i - camera.f, j - camera.f, camera.f, 1);
      dir = camera.c2w * dir;

      /* Checks if ray intersects */
      intersects = ClosestIntersection(
        camera.GetCameraPos(),
        dir,
        triangles,
        intersection,
        false);

      /* If intersection occurs, draw a pixel */
      if (intersects) {
        
        switch(camera.colorMode) {
            case 0:
                // Normal mode
                colour = triangles[intersection.triangleIndex].color;
                colour *= DirectLight(intersection, lightSource, camera, triangles);
                //colour = vec3(camera.c2w * vec4(colour,0));
                colour += 0.2f*triangles[intersection.triangleIndex].color;
                break;
            case 1:
                // Grayscale mode
                colour = vec3(1,1,1);
                colour *= DirectLight(intersection, lightSource, camera, triangles);
                colour += 0.2f*vec3(1,1,1);
                break;
            case 2: {
                // Funky night camera mode
                float r = ((double) rand() / (RAND_MAX)) + 1;
                float g = ((double) rand() / (RAND_MAX)) + 1;
                float b = ((double) rand() / (RAND_MAX)) + 1;
                colour = vec3(r,g,b);
                colour = cross((float)0.3*colour, triangles[intersection.triangleIndex].color);
                colour = vec3(camera.c2w * vec4(colour,0));
                colour += 0.2f*vec3(1,1,1);
            } break;
            default:
                // Normal mode
                colour = triangles[intersection.triangleIndex].color;
                break;
        }
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
    Intersection& closestIntersection,
    bool isShadowRay) {

  bool intersects = false;

  closestIntersection.position = start;
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

    /* Vector from start to corner of triangle */
    mat3 A(-(vec3)dir, e1, e2);
    /* The intersection point of the plane and ray */
    vec3 x = glm::inverse(A) * b;  

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
      /* Return on first intersection, shaves down 4ms lol */
      if(isShadowRay) return intersects;

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
vec3 DirectLight (const Intersection& i, LightSource &source, Camera &camera, const std::vector<Triangle>& triangles){

    /* Illumination */
    /* Vector between intersection point and light source */
    vec4 dir = vec4(source.pos - i.position);
    /* Distance / magnitude */
    float rMag = glm::length(dir);
    /* Direction normal */
    vec4 rHat = glm::normalize(dir);
    /* Intersecting triangle normal */
    vec4 nHat = triangles[i.triangleIndex].normal;
    /* lol */
    const float PI4 = 12.5663706144;
    /* Projection of normals */
    float cosTheta = std::max(dot(rHat, nHat), 0.f);
    /* Power per real surface */
    vec3 D = source.color * source.intensity * cosTheta /
      (PI4*rMag*rMag);

    /* Specular */
    /* Vector between intersection point and camera */
    vec4 sDir = vec4(camera.GetCameraPos() - i.position);
    /* Distance / magnitude */
    float srMag = glm::length(sDir);
    /* Direction normal */
    vec4 srHat = glm::normalize(sDir);
    /* Projection of normals */
    float cosAlpha = std::max(dot(srHat, nHat), 0.f);
    /* Power per real surface */
    D = (source.color * std::pow(cosAlpha, 10.0f) / (PI4*srMag*srMag)) + 0.8f*D;

    /* Shadows */
    /* Bias to fix 'shadow-acne', lol */
    float bias = 0.01f;
    /* Shadow ray start position */ 
    vec4 start = i.position + rHat * bias;
    /* Intersection with occluding object */
    Intersection intersection;
    /* Finding the nearest intersection */
    bool intersects = false;
    intersects = (ClosestIntersection(
        start,
        dir,
        triangles,
        intersection,
        true));
    /* If the intersection occludes light */
    if(intersects && intersection.distance <= rMag) {
        D = vec3(0,0,0);
    }

    return D;
}
