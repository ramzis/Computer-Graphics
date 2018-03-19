#include "rasteriser.h"
#include <omp.h>


#define SCREEN_WIDTH 320*2
#define SCREEN_HEIGHT 256*2
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

  vector <vec4> vertices(3);
  /* Temp buffer for parallel */
  uint32_t *buff = new uint32_t[screen->height*screen->width];

  //omp_set_num_threads(4);
  //#pragma omp parallel default(none) private(vertices) shared(buff, triangles)
  //#pragma omp parallel for
  for (uint32_t i=0; i < triangles.size(); ++i) {

    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;

    DrawPolygonEdges(screen, vertices);
    //BufferPolygonEdges(buff, vertices);

    /* Testing DrawLineSDL */
    // vec3 colour(1.,1.,1.);
    // ivec2 top(0, 0);
    // ivec2 bot(50, 50);
    // DrawLineSDL(screen, top, bot, colour);
    // top = ivec2(50, 0);
    // bot = ivec2(0, 50);
    // DrawLineSDL(screen, top, bot, colour);

    /* Draw only vertices */
    // for (int v=0; v<3; ++v) {
    //   ivec2 projPos;
    //   VertexShader(vertices[v], projPos);
    //   vec3 colour(1,.1,.8);
    //   PutPixelSDL(screen, projPos.x, projPos.y, colour);
    //   PutPixelSDL(screen, projPos.x-1, projPos.y, colour);
    //   PutPixelSDL(screen, projPos.x+1, projPos.y, colour);
    //   PutPixelSDL(screen, projPos.x, projPos.y-1, colour);
    //   PutPixelSDL(screen, projPos.x, projPos.y+1, colour);
    // }
  }

  // for (int i = 0; i < screen->height*screen->width; i++){
  //     screen->buffer[i] = buff[i];
  // }
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


void VertexShader(const vec4& v, ivec2& p){
  
  // TODO: import real camera.
  vec4 cameraPos(0, 0, -3.001, 1);
  float f = 400.0f;
  
  vec4 v_1 = v - cameraPos;
  p.x = f * v_1.x / v_1.z + SCREEN_WIDTH/2;
  p.y = f * v_1.y / v_1.z + SCREEN_HEIGHT/2;

}


//TODO: write this more generally? so that it works with any dimensions of vectors and not just ivec2??
//TODO: look into using Breshenham instead of this linear interpolation??
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result){
  
  int N = result.size();
  vec2 step = vec2(b-a) / float(max(N-1,1));
  vec2 current(a);

  for( int i=0; i<N; i++){
    result[i] = current;
    current += step;
  }

}


void DrawLineSDL(screen* surface, ivec2 a, ivec2 b, vec3 colour){
  
  ivec2 delta = glm::abs(a-b);
  int pixels = glm::max(delta.x, delta.y) + 1;
  vector<ivec2> line(pixels);
  Interpolate(a, b, line);
  for (uint i = 0; i < line.size(); i++)
  {
    PutPixelSDL(surface, line[i].x, line[i].y, colour);
  }
}


void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices){
  
  vec3 color ( 1, 1, 1 );

  /* Transform each vertex from 3D world position to 2D image position */
  int V = vertices.size();
  vector<ivec2> projectedVertices(V);
  for (int i = 0; i < V; i++){
    VertexShader(vertices[i], projectedVertices[i]);
  }

  DrawLineSDL(screen, projectedVertices[0], projectedVertices[1], color);
  DrawLineSDL(screen, projectedVertices[1], projectedVertices[2], color);
  DrawLineSDL(screen, projectedVertices[2], projectedVertices[0], color);
}


void DrawLineBuffer(uint32_t* buff, ivec2 a, ivec2 b, vec3 colour) {

  ivec2 delta = glm::abs(a-b);
  int pixels = glm::max(delta.x, delta.y) + 1;
  vector<ivec2> line(pixels);
  Interpolate(a, b, line);
  for (uint i = 0; i < line.size(); i++)
  {
    #pragma omp atomic write
    buff[line[i].y*SCREEN_WIDTH + line[i].x] = 
      (255<<24) + ((int)colour.x<<16) + ((int)colour.x<<8) + (int)colour.z;
  }
}

void BufferPolygonEdges(uint32_t* buff, const vector<vec4>& vertices){
  
  vec3 color ( 1, 1, 1 );
  /* Transform each vertex from 3D world position to 2D image position */
  int V = vertices.size();
  vector<ivec2> projectedVertices(V);
  for (int i = 0; i < V; i++){
    VertexShader(vertices[i], projectedVertices[i]);
  }

  DrawLineBuffer(buff, projectedVertices[0], projectedVertices[1], color);
  DrawLineBuffer(buff, projectedVertices[1], projectedVertices[2], color);
  DrawLineBuffer(buff, projectedVertices[2], projectedVertices[0], color);
}
