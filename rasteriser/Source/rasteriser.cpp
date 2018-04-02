#include "rasteriser.h"
#include <omp.h>


#define SCREEN_WIDTH 320*2
#define SCREEN_HEIGHT 256*2
#define FULLSCREEN_MODE true


////////////////////////////////////////////////////////////////////////////////
//
// Initializes buffers and starts the engine loop.
//
////////////////////////////////////////////////////////////////////////////////
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

  triangles.clear();

  SDL_SaveImage( screen, "screenshot.bmp" );

  KillSDL(screen);
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
//
// Updates variable values every frame.
//
////////////////////////////////////////////////////////////////////////////////
void Update()
{
  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/
}


////////////////////////////////////////////////////////////////////////////////
//
// Draws the frame by rasterizing the vertices of objects in the scene.
//
////////////////////////////////////////////////////////////////////////////////
void Draw(screen* screen, vector<Triangle>& triangles) {

  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  memset(screen->depthBuffer, 0, screen->height*screen->width*sizeof(float));

  vector<vec4> vertices(3);

  //omp_set_num_threads(4);
  //#pragma omp parallel default(none) private(vertices) shared(buff, triangles)
  //#pragma omp parallel for
  for (uint32_t i=0; i < triangles.size(); ++i) {

    vertices[0] = triangles[i].v0;
    vertices[1] = triangles[i].v1;
    vertices[2] = triangles[i].v2;

    // Not implemented
    //BufferPolygonEdges(buff, vertices);
    // No depth old
    //DrawPolygon(screen, vertices, triangles[i].color);
    // New with depth
    DrawPolygonDepth(screen, vertices, triangles[i].color);
    // Optionally show edges
    //DrawPolygonEdges(screen, vertices);

  }

}


////////////////////////////////////////////////////////////////////////////////
//
// Draws a 3D polygon onto the screen.
//
////////////////////////////////////////////////////////////////////////////////
void DrawPolygonDepth(
  screen* screen,
  const vector<vec4>& vertices,
  vec3 colour) {
  
  /* Transform each vertex from 3D world position to 2D image position */
  int V = vertices.size();
  vector<Pixel> vertexPixels(V);
  for(int i=0; i<V; i++)
    VertexShader(vertices[i], vertexPixels[i]);
  
  /* Find the rows that make up the polygon */
  vector<Pixel> leftPixels, rightPixels;
  ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
  /* Draw the rows that make up the polygon */
  for (uint i = 0; i < leftPixels.size(); i++)
    DrawLineSDL(screen, leftPixels[i], rightPixels[i], colour);
}


////////////////////////////////////////////////////////////////////////////////
//
// Creates a perspective projection of a 3D polygon onto the camera plane.
// Calculates the inverse depth of each vertex.
//
////////////////////////////////////////////////////////////////////////////////
// TODO: use real camera.
void VertexShader(const vec4& v, Pixel& p) {

  vec4 cameraPos(0, 0, -3., 1);
  float f = 400.0f;
  
  vec4 v_1 = v - cameraPos;
  p.x = f * v_1.x / v_1.z + SCREEN_WIDTH/2;
  p.y = f * v_1.y / v_1.z + SCREEN_HEIGHT/2;
  if(v_1.z != 0) p.zinv = 1 / v_1.z;
}


////////////////////////////////////////////////////////////////////////////////
//
// Linearly interpolates between two Pixel values.
//
////////////////////////////////////////////////////////////////////////////////
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result) {

  int N = result.size();
  Pixel d = b-a;
  vec3 step = vec3(d.x, d.y, d.zinv) / float(max(N-1,1));
  vec3 current = vec3(a.x, a.y, a.zinv);

  for(int i=0; i<N; i++){
    result[i] = Pixel(current.x, current.y, current.z);
    current += step;
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draws a line on the screen. 
//
////////////////////////////////////////////////////////////////////////////////
void DrawLineSDL(screen* screen, Pixel a, Pixel b, vec3 colour) {
  
  Pixel delta = (a-b).abs();
  int pixels = glm::max(delta.x, delta.y) + 1;
  vector<Pixel> line(pixels);
  Interpolate(a, b, line);
  for (uint i = 0; i < line.size(); i++){
    if(screen->depthBuffer[screen->width*line[i].y + line[i].x] < line[i].zinv - 0.001f){
      PutPixelSDL(screen, line[i].x, line[i].y, colour);
      screen->depthBuffer[screen->width*line[i].y + line[i].x] = line[i].zinv;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Computes the start and end coordinates of each row of a polygon.
//
////////////////////////////////////////////////////////////////////////////////
void ComputePolygonRows(
  const vector<Pixel>& vertexPixels, 
  vector<Pixel>& leftPixels, 
  vector<Pixel>& rightPixels) {

  // 1. Find max and min y-value of the polygon
  // and compute the number of rows it occupies.
  int minY = +numeric_limits<int>::max(), 
      maxY = -numeric_limits<int>::max();

  for (uint i = 0; i < vertexPixels.size(); ++i)
  {
    if(vertexPixels[i].y < minY)
      minY = vertexPixels[i].y;
    if(vertexPixels[i].y > maxY)
      maxY = vertexPixels[i].y;
  }

  const int rows = maxY - minY + 1;

  // 2. Resize leftPixels and rightPixels
  // so that they have an element for each row.
  leftPixels  = vector<Pixel>(rows);
  rightPixels = vector<Pixel>(rows);

  // 3. Initialize the x-coordinates in leftPixels
  // to some really large value and the x-coordinates
  // in rightPixels to some really small value.
  for( int i=0; i<rows; ++i )
  {
    leftPixels[i].x = +numeric_limits<int>::max();
    rightPixels[i].x = -numeric_limits<int>::max();
  }
  
  // 4. Loop through all edges of the polygon and use
  // linear interpolation to find the x-coordinate for
  // each row it occupies. Update the corresponding
  // values in rightPixels and leftPixels.
  int vertices = vertexPixels.size();
  for(int i=0; i<vertices; i++){
    Pixel v0 = vertexPixels[i];
    Pixel v1 = vertexPixels[i+1>=vertices?0:i+1]; // essentially mod
    // warning: magic below
    int results = abs(v0.y-v1.y) + 1;
    vector<Pixel> result(results);
    Interpolate(v0, v1, result);
    for (int r = 0; r < results; r++)
    {
      if(result[r].x < leftPixels[result[r].y - minY].x){
        leftPixels[result[r].y - minY].x = result[r].x;
        leftPixels[result[r].y - minY].y = result[r].y;
        leftPixels[result[r].y - minY].zinv = result[r].zinv;
      }
      if(result[r].x > rightPixels[result[r].y - minY].x){
        rightPixels[result[r].y - minY].x = result[r].x;
        rightPixels[result[r].y - minY].y = result[r].y;
        rightPixels[result[r].y - minY].zinv = result[r].zinv;
      }
    }
  }
}


/*===========================
 * WORKING POLYGON NO DEPTH 
 * FUNCTIONS KEPT AS ARCHIVE
 *===========================*/
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


void VertexShader(const vec4& v, ivec2& p){
  
  vec4 cameraPos(0, 0, -3.001, 1);
  float f = 400.0f;
  
  vec4 v_1 = v - cameraPos;
  p.x = f * v_1.x / v_1.z + SCREEN_WIDTH/2;
  p.y = f * v_1.y / v_1.z + SCREEN_HEIGHT/2;
}


void DrawPolygon(screen* screen, const vector<vec4>& vertices, vec3 colour) {
  
  /* Transform each vertex from 3D world position to 2D image position */
  int V = vertices.size();
  vector<ivec2> vertexPixels(V);
  for(int i=0; i<V; i++)
    VertexShader(vertices[i], vertexPixels[i]);
  
  vector<ivec2> leftPixels;
  vector<ivec2> rightPixels;
  ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
  DrawPolygonRows(screen, leftPixels, rightPixels, colour);
}


void DrawPolygonRows(
  screen* screen,
  const vector<ivec2>& leftPixels, 
  const vector<ivec2>& rightPixels,
  vec3 colour) {

  for (uint i = 0; i < leftPixels.size(); i++)
    DrawLineSDL(screen, leftPixels[i], rightPixels[i], colour);
}


void ComputePolygonRows(
  const vector<ivec2>& vertexPixels,
  vector<ivec2>& leftPixels,
  vector<ivec2>& rightPixels) {

  // 1. Find max and min y-value of the polygon
  // and compute the number of rows it occupies.
  int minY = +numeric_limits<int>::max(), 
      maxY = -numeric_limits<int>::max();

  for (uint i = 0; i < vertexPixels.size(); ++i)
  {
    if(vertexPixels[i].y < minY)
      minY = vertexPixels[i].y;
    if(vertexPixels[i].y > maxY)
      maxY = vertexPixels[i].y;
  }

  const int rows = maxY - minY + 1;

  // 2. Resize leftPixels and rightPixels
  // so that they have an element for each row.
  leftPixels  = vector<ivec2>(rows);
  rightPixels = vector<ivec2>(rows);

  // 3. Initialize the x-coordinates in leftPixels
  // to some really large value and the x-coordinates
  // in rightPixels to some really small value.
  for( int i=0; i<rows; ++i )
  {
    leftPixels[i].x = +numeric_limits<int>::max();
    rightPixels[i].x = -numeric_limits<int>::max();
  }
  
  // 4. Loop through all edges of the polygon and use
  // linear interpolation to find the x-coordinate for
  // each row it occupies. Update the corresponding
  // values in rightPixels and leftPixels.
  int vertices = vertexPixels.size();
  for(int i=0; i<vertices; i++){
    ivec2 v0 = vertexPixels[i];
    ivec2 v1 = vertexPixels[i+1>=vertices?0:i+1]; // essentially mod
    // warning: magic below
    int results = abs(v0.y-v1.y) + 1;
    vector<ivec2> result(results);
    Interpolate(v0, v1, result);
    for (int r = 0; r < results; r++)
    {
      if(result[r].x < leftPixels[result[r].y - minY].x){
        leftPixels[result[r].y - minY].x = result[r].x;
        leftPixels[result[r].y - minY].y = result[r].y;
      }
      if(result[r].x > rightPixels[result[r].y - minY].x){
        rightPixels[result[r].y - minY].x = result[r].x;
        rightPixels[result[r].y - minY].y = result[r].y;
      }
    }
  }
}


/*===========================
 * TEST FUNCTIONS
 *===========================*/

void test() {
  
  vector<ivec2> vertexPixels(3);
  vertexPixels[0] = ivec2(10, 5);
  vertexPixels[1] = ivec2( 5,10);
  vertexPixels[2] = ivec2(15,15);
  vector<ivec2> leftPixels;
  vector<ivec2> rightPixels;
  
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );
  
  for( uint row=0; row<leftPixels.size(); ++row )
  {
    cout << "Start: ("
    << leftPixels[row].x << ","
    << leftPixels[row].y << "). "
    << "End: ("
    << rightPixels[row].x << ","
    << rightPixels[row].y << "). " << endl;
  }
}

void test2(){

  vector<ivec2> vertexPixels(3);
  vertexPixels[0] = ivec2(10, 5);
  vertexPixels[1] = ivec2( 5,10);
  vertexPixels[2] = ivec2(15,15);
    
  ivec2 v0 = vertexPixels[1];
  ivec2 v1 = vertexPixels[2];
  vector<ivec2> result(6);
  Interpolate(v0, v1, result);
  for( uint row=0; row<result.size(); ++row )
  {
    cout << "("
    << result[row].x << ","
    << result[row].y << "). " << endl;
  }
}

void test3() {
  
  vector<Pixel> vertexPixels(3);
  vertexPixels[0] = Pixel(10, 5, 1.f);
  vertexPixels[1] = Pixel( 5,10, 5.f);
  vertexPixels[2] = Pixel(15,15, 2.f);
  vector<Pixel> leftPixels;
  vector<Pixel> rightPixels;
  
  ComputePolygonRows( vertexPixels, leftPixels, rightPixels );

  for( uint row=0; row<leftPixels.size(); ++row )
  {
    cout << "Start: ("
    << leftPixels[row].x << ","
    << leftPixels[row].y << ","
    << leftPixels[row].zinv << "). "
    << "End: ("
    << rightPixels[row].x << ","
    << rightPixels[row].y << ","
    << rightPixels[row].zinv << "). " << endl;
  }
}

void test4(){

  vector<Pixel> vertexPixels(3);
  vertexPixels[0] = Pixel(10, 5, 0.f);
  vertexPixels[1] = Pixel( 5,10, 5.f);
  vertexPixels[2] = Pixel(15,15, 6.f);
    
  Pixel v0 = vertexPixels[0];
  Pixel v1 = vertexPixels[2];
  vector<Pixel> result(6);
  Interpolate(v0, v1, result);
  for( uint row=0; row<result.size(); ++row )
  {
    cout << "("
    << result[row].x << ","
    << result[row].y << ","
    << result[row].zinv << "). " << endl;
  }
}

void test5(screen *screen){

  memset(screen->depthBuffer, 0, screen->height*screen->width*sizeof(float));

  vector<Pixel> vertexPixels(4);
  vertexPixels[0] = Pixel(1, 1, 0.2f);
  vertexPixels[1] = Pixel(50,50, 0.3f);
  vertexPixels[2] = Pixel(1,50, 2.f);
  vertexPixels[3] = Pixel(50,1, 6.f);

  vec3 colour = vec3(1.,1.,1.);

  DrawLineSDL(screen, vertexPixels[0], vertexPixels[1], colour);
  DrawLineSDL(screen, vertexPixels[2], vertexPixels[3], colour);
}
