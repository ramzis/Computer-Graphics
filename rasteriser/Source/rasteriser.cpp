#include "rasteriser.h"


#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 512
#define FULLSCREEN_MODE true



int main( int argc, char* argv[] )
{

  screen *screen = InitializeSDL( SCREEN_WIDTH, SCREEN_HEIGHT, FULLSCREEN_MODE );

  /* Model data init */
  vector<Triangle> triangles;
  LoadTestModel(triangles);

  while( NoQuitMessageSDL() )
    {
      Update();
      Draw(screen, triangles);
      SDL_Renderframe(screen);
    }

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}

/*Place your drawing here*/
void Draw(screen* screen, std::vector<Triangle>& triangles)
{
  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));

  for (uint32_t i=0; i<triangles.size(); ++i) {
    vector <vec4> vertices(3);//move this outside for loop?

    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;

    DrawPolygonEdges(vertices);

    for (int v=0; v<3; ++v) {
      ivec2 projPos;
      VertexShader( vertices[v], projPos);
      vec3 colour(1,.1,.8);
      PutPixelSDL(screen, projPos.x, projPos.y, colour);

            
    }//speed things up by removing this loop to get rid of overhead of creating a loop?modulo divs are heavy, so would be preferable to have that outside the loop


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



void VertexShader( const vec4& v, ivec2& p ){
  float f = 5;//TODO pull in camera.f TODO fix this :p
  //do -camera pos + v then use those x,y,zs???
  p.x = f * v.x / v.z + SCREEN_WIDTH/2;
  p.y = f * v.y / v.z + SCREEN_HEIGHT/2;

}


void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result){
  int N = result.size();
  vec2 step = vec2(b-1)/float(max(N-1,1));
  vec2 current(a);

  for( int i=0; i<N; i++){
    result[i] = current;
    current += step;
  }
//TODO: write this more generally? so that it works with any dimensions of vectors and not just ivec2??
  //TODO: look into using Breshenham instead of this linear interpolation??
}

void DrawLineSDL( SDL_Surface* surface, ivec2 a, ivec2 b, vec3 colour){
  ivec2 delta = glm::abs(a-b);
  int pixels = glm::max(delta.x, delta.y) +1;
  vector<ivec2> line(pixels);
  Interpolate(a, b, line);
}

void DrawPolygonEdges(const vector<vec4>& vertices){
  int V = vertices.size();

  //Transform each vertex from 3D world position to 2D image position:
  vector<ivec2> projectedVertices( V );
  for( int i=0; i<V; ++i)
  {
    int j = (i+1)%V; // the next vertex
    vec3 color ( 1, 1, 1 );
    //DrawLineSDL( screen, projectedVertices[i], projectedVertices[j], color );
  }
}