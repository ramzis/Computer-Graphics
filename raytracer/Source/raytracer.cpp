#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <math.h>//only need fabs

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
//using glm::distance;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 256
#define FULLSCREEN_MODE false

struct Intersection {
  vec4 position;
  float distance;
  int triangleIndex;
};


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen);
void DrawRoom(screen* screen, vector<Triangle>& triangles, int focalLength);
bool ClosestIntersection( vec4 start, vec4 dir, const vector<Triangle>& triangles, Intersection& closestIntersection);
void buildCameraRay(int i,int j,vec4& start,vec4& dir);
float distance (vec4& i, vec4& j);

int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  vector<Triangle> triangles;
  LoadTestModel( triangles);
  //vec4 cameraOrigin;
  float f = screen->height/8;

  while( NoQuitMessageSDL() )
    {
      Update();
      //Draw(screen);
      DrawRoom(screen, triangles, f);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

void DrawRoom(screen* screen, vector<Triangle>& triangles,int  focalLength){
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  bool intersects = false;
  Intersection intersection;
  vec3 col;
  vec4 d;

  for (int i = 0; i<screen->width; i++){
    for (int j= 0; j<screen->height; j++){
      //compute ray direction
      d = vec4(i-screen->width/2, j-screen->width/2, focalLength,1);
      //check if ray intersects
      intersects = ClosestIntersection(vec4(0,0,0,1), d, triangles, intersection);
      if (intersects){
        col = triangles[intersection.triangleIndex].color;
        PutPixelSDL(screen, i, j, col);
      }
    }
  }
}


/*Place your drawing here*/
void Draw(screen* screen)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  vec3 colour(1.0,0.0,0.0);
  for(int i=0; i<1000; i++)
    {
      uint32_t x = rand() % screen->width;
      uint32_t y = rand() % screen->height;
      PutPixelSDL(screen, x, y, colour);
    }
}

/*Place updates of parameters here*/
void Update()
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
}


//@param start: it's the starting poitn of the ray vector
//@param dir: the direction the ray vector is travelling
bool ClosestIntersection(//v0+ue1+ve2=s+td
  vec4 start,
  vec4 dir,
  const vector<Triangle>& triangles,
  Intersection& closestIntersection) {

  bool intersects = false;
  closestIntersection.position = vec4(1.1,1.1,1.1,1);
  closestIntersection.distance = 100.0;
  for (int i=0; i< triangles.size(); i++){
      Triangle triangle = triangles[i];
      //get points of triangle
      vec4 v0 = triangle.v0;
      vec4 v1 = triangle.v1;
      vec4 v2 = triangle.v2;
      //gen the plane this triangle is in and vector from start to triangle
      vec3 e1 = vec3(v1.x-v0.x,v1.y-v0.y,v1.z-v0.z);//basis vector of triangle
      vec3 e2 = vec3(v2.x-v0.x,v2.y-v0.y,v2.z-v0.z);//basis vector of triangle
      vec3 b = vec3(start.x-v0.x,start.y-v0.y,start.z-v0.z);//vector from start to corner of triangle

      //mat3 A( (float)-1 * (vec3)dir, e1, e2);
      mat3 A (- (vec3)dir, e1, e2);
      vec3 x = glm::inverse(A)*b;//x is the intersection point of the plane and ray
      //x=(t,u,v)

      //check intersection is inside triangle boundaries
      bool seven = x.y > 0;
      bool eight = x.z > 0;
      bool nine = x.y+x.z < 1;
      bool eleven = x.x >=0;
      if (seven && eight && nine && eleven){
        intersects = true;

        if (x.x < closestIntersection.distance){
          closestIntersection.position = vec4(b.x, b.y, b.z, 1);//2.bmp
          //closestIntersection.position = vec4(v0.x, v0.y, v0.z, 1);//1.bmp
          //closestIntersection.position.x = b.x;
          closestIntersection.distance = x.x;
          closestIntersection.triangleIndex = i;
        }
      }

     //need to check that new intersecting points are closer than previous
     /*if (intersects && x.x < closestIntersection.distance) {//NOTE:the second part of this if isn't actually doing anything
       closestIntersection.position = vec4(b.x, b.y, b.z, 1);
       //closestIntersection.position.x = b.x;
       closestIntersection.distance = x.x;
       closestIntersection.triangleIndex = i;
    }*/

  }
  return intersects;
}

/*float distance(vec4 i, vec4 j){
  float x = i.x-j.x;
  float y = i.y-j.y;
  float z = i.z-j.z;
  float dist = sqrt(x*x + y*y + z*z);
  return dist;
}*/

void buildCameraRay(int i,int j,vec4& start,vec4& dir){
  start = vec4(0,0,0,1);
}
