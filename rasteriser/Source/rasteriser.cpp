#include "rasteriser.h"
#include <omp.h>


#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 500
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

  /* Camera init */
  /* Position in world coordinates */
  // NOTE: z=0 is the back wall and +ve z is towards the camera
  vec4 camPos = vec4(0.f,0.f,2.0f,1.0f);
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
  vec4 lightPos = vec4(0.5f,0.5f,1.5f, 1);
  /* Light colour */
  vec3 lightColour = vec3(1, 1, 1);
  /* Light intensity */
  float lightIntensity = 60.0f;
  LightSource light = LightSource(
    lightPos,
    lightColour,
    lightIntensity);

  while( NoQuitMessageSDL() )
  {
    Update(camera, light);
    Draw(screen, camera, triangles, light);
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
void Update(Camera &camera, LightSource &light)
{
  const Uint8* keystate = SDL_GetKeyboardState(0);

  static int t = SDL_GetTicks();
  /* Compute frame time */
  int t2 = SDL_GetTicks();
  float dt = float(t2-t);
  t = t2;
  /*Good idea to remove this*/
  //std::cout << "Render time: " << dt << " ms." << std::endl;
  /* Update variables*/

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
// Updates the position of the camera.
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
  if(keystate[SDL_SCANCODE_Q]) {
    cameraTranslate.y = cameraSpeed;
  }
  if(keystate[SDL_SCANCODE_E]) {
    cameraTranslate.y = -cameraSpeed;
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

  //TODO: add camera colour modes
  if(keystate[SDL_SCANCODE_1]) {
    camera.colorMode = 0;
  }
  if(keystate[SDL_SCANCODE_2]) {
    camera.colorMode = 1;
  }
}

////////////////////////////////////////////////////////////////////////////////
//
// Draws the frame by rasterizing the vertices of objects in the scene.
//
////////////////////////////////////////////////////////////////////////////////
void Draw(screen* screen, Camera &camera, vector<Triangle>& triangles, LightSource &light) {

  /* Clear buffer */
  memset(screen->buffer, 0, screen->height*screen->width*sizeof(uint32_t));
  memset(screen->depthBuffer, 0, screen->height*screen->width*sizeof(float));
  memset(screen->shadowBuffer, 0, screen->height*screen->width*sizeof(float));

  for (uint32_t i=0; i < triangles.size(); ++i) {
    DrawPolygonDepth(screen, camera, triangles[i], light, true, false);
  }
  for (uint32_t i=0; i < triangles.size(); ++i) {
    DrawPolygonDepth(screen, camera, triangles[i], light, false, true);
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draws a 3D polygon onto the screen.
//
////////////////////////////////////////////////////////////////////////////////
void DrawPolygonDepth(screen* screen, Camera &camera, const Triangle &t,
  LightSource &light, bool depthOnly, bool isCamera) {

  /* Transform each 3D Vertex to a 2D Pixel */
  vector<Pixel> vertexPixels(3);
  if(!isCamera)
  { 
    VertexShaderLight(camera, light, Vertex(t.v0), vertexPixels[0]);
    VertexShaderLight(camera, light, Vertex(t.v1), vertexPixels[1]);
    VertexShaderLight(camera, light, Vertex(t.v2), vertexPixels[2]);
  }
  else
  {
    VertexShader(camera, light, Vertex(t.v0), vertexPixels[0]);
    VertexShader(camera, light, Vertex(t.v1), vertexPixels[1]);
    VertexShader(camera, light, Vertex(t.v2), vertexPixels[2]);
  }

  /* Find the rows that make up the polygon */
  vector<Pixel> leftPixels, rightPixels;
  ComputePolygonRows(vertexPixels, leftPixels, rightPixels);
  
  /*Color Mode*/
  vec3 color = t.color;
  switch(camera.colorMode){
    case 0:
      //Normal Mode
      color = t.color;
      break;
    case 1:
      //invert color
      color = 1.f - t.color;
      break;
    default:
      color = t.color;
      break;
  }

  /* Draw the rows that make up the polygon */
  for (uint i = 0; i < leftPixels.size(); i++)
    DrawLineSDL(screen, camera, light, leftPixels[i], rightPixels[i],
      t.normal, color, t.reflectance, depthOnly);
}


////////////////////////////////////////////////////////////////////////////////
//
// Creates a perspective projection of a Vertex onto the camera plane.
// Calculates the inverse depth of each vertex.
//
////////////////////////////////////////////////////////////////////////////////
void VertexShader(Camera &camera, LightSource &light, const Vertex& v, Pixel& p) {

  glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );

  vec4 pos = v.pos - 2.0f * camera.GetCameraPos();
  pos = camera.c2w * pos;
  p.zinv = 1 / (pos.z == 0 ? 0.0001f : pos.z);
  p.x = int(camera.f * pos.x * p.zinv) + SCREEN_WIDTH/2;
  p.y = int(camera.f * pos.y * p.zinv) + SCREEN_HEIGHT/2;
  p.pos3d = pos;

}


////////////////////////////////////////////////////////////////////////////////
//
// Creates an orthographic projection of a Vertex onto the shadow map.
// Calculates the inverse depth of each vertex.
//
////////////////////////////////////////////////////////////////////////////////
void VertexShaderLight(Camera &camera, LightSource &light, const Vertex& v, Pixel& p) {

  glm::mat4 biasMatrix(
    0.5, 0.0, 0.0, 0.0,
    0.0, 0.5, 0.0, 0.0,
    0.0, 0.0, 0.5, 0.0,
    0.5, 0.5, 0.5, 1.0
    );

  glm::vec3 lightInvDir = glm::vec3(light.pos);

  // Compute the MVP matrix from the light's point of view
  glm::mat4 depthProjectionMatrix = glm::ortho<float>(-2,2,-2,2,10,1);
  glm::mat4 depthViewMatrix = glm::lookAt(lightInvDir, glm::vec3(0,0,0), glm::vec3(0,1,0));
  glm::mat4 depthModelMatrix = glm::mat4(1.0);
  glm::mat4 depthMVP = depthProjectionMatrix * depthViewMatrix * depthModelMatrix;
  glm::mat4 depthBiasMVP = biasMatrix * depthMVP;

  vec4 pos = depthBiasMVP * v.pos;
  p.zinv = 1 / (pos.z == 0 ? 0.0001f : pos.z);
  p.x = pos.x * camera.f * 2;
  p.y = pos.y * camera.f * 2;
  p.pos3d = v.pos;
}


////////////////////////////////////////////////////////////////////////////////
//
// Performs per-pixel shading and draws a pixel to the screen.
//
////////////////////////////////////////////////////////////////////////////////
void PixelShader(screen* screen, Camera &camera, LightSource &light, const Pixel& p,
  const vec4 &normal, const vec3 &colour, const vec3 &reflectance, const float &visibility) {

  float vis = (screen->shadowBuffer[screen->width*p.y + p.x]) < glm::length(light.pos - p.pos3d) ? 0.75f : 1.0f; 

  vec3 illumination = colour * vis * DirectLight(camera, light, p, normal, reflectance);

  if(p.x < screen->width && p.x >=0 && p.y < screen->height && p.y >=0){
    if(screen->depthBuffer[screen->width*p.y + p.x] < p.zinv - 0.001f) {
      PutPixelSDL(screen, p.x, p.y, illumination);
      screen->depthBuffer[screen->width*p.y + p.x] = p.zinv;
    }
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Performs shading only on the depth buffer.
//
////////////////////////////////////////////////////////////////////////////////
void FragmentShader(screen* screen, const Pixel& p) {
  if(screen->depthBuffer[screen->width*p.y + p.x] < p.zinv - 0.001f) {
      float zNear = 10.f;  
      float zFar  = -10.0f;
      float depth = 1.f/p.zinv;
      float c = (2.0f * zNear) / (zFar + zNear - depth * (zFar - zNear));
    if(p.x < screen->width && p.x >=0 && p.y < screen->height && p.y >=0){
      PutPixelSDL(screen, p.x, p.y, vec3(c, c, c));
      screen->shadowBuffer[screen->width*p.y + p.x] = p.zinv;
    }
  }
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

  //vec4 posStep = d.pos3d / float(max(N-1,1));
  //vec4 currentPos = a.pos3d;

  vec4 diff = vec4(
    b.pos3d.x * b.zinv - a.pos3d.x * a.zinv,
    b.pos3d.y * b.zinv - a.pos3d.y * a.zinv,
    b.pos3d.z - a.pos3d.z,
    b.pos3d.w - a.pos3d.w);

  vec4 posStep = diff / (float(max(N-1,1)));

  vec4 currentPos = vec4(
    a.pos3d.x * a.zinv,
    a.pos3d.y * a.zinv,
    a.pos3d.z,
    a.pos3d.w);

  for(int i=0; i<N; i++){
    result[i] = Pixel(current.x, current.y, current.z, vec4(currentPos.x/current.z,currentPos.y/current.z,currentPos.z,currentPos.w));
    current += step;
    currentPos += posStep;
  }
}


////////////////////////////////////////////////////////////////////////////////
//
// Draws a line on the screen between two Pixel values.
//
////////////////////////////////////////////////////////////////////////////////
void DrawLineSDL(screen* screen, Camera &camera, LightSource &light, Pixel& a, Pixel& b,
  const vec4 &normal, const vec3 &colour, const vec3 &reflectance,
  bool depthOnly) {

  Pixel delta = (a-b).abs();
  int pixels = glm::max(delta.x, delta.y) + 1;
  vector<Pixel> line(pixels);
  Interpolate(a, b, line);
  for (int i = 0; i < pixels; i++){
    if(!depthOnly)
      PixelShader(screen, camera, light, line[i], normal, colour, reflectance, 1.0f);
    else
      FragmentShader(screen, line[i]);
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
    // warning: Here be dragons
    int results = abs(v0.y-v1.y) + 1;
    vector<Pixel> result(results);
    Interpolate(v0, v1, result);
    for (int r = 0; r < results; r++)
    {
      if(result[r].x < leftPixels[result[r].y - minY].x)
        leftPixels[result[r].y - minY] = result[r];
      if(result[r].x > rightPixels[result[r].y - minY].x)
        rightPixels[result[r].y - minY] = result[r];
    }
  }
}

// TODO: use reflectance, use real camera.
vec3 DirectLight(Camera &camera, LightSource &light, const Pixel &p, const vec4 &normal, const vec3 &reflectance){

    vec4 lightPos = light.pos;
    lightPos -= 2.0f * camera.GetCameraPos();
    lightPos = camera.c2w * lightPos;
    //TODO:remove the change of the camera's rotation
    vec3 lightPower = 20.1f*vec3( 1, 1, 1 );
    vec3 indirectLightPowerPerArea = 0.5f*vec3(1.f, 1.f, 1.f);

    /* Illumination */
    /* Vector between intersection point and light source */
    vec4 dir = lightPos - p.pos3d;
    /* Distance / magnitude */
    float rMag = glm::length(dir);
    /* Direction normal */
    vec4 rHat = glm::normalize(dir);
    /* Vertex normal */
    vec4 nHat = normal;
    /* lol */
    const float PI4 = 12.5663706144;
    /* Projection of normals */
    float cosTheta = std::max(dot(rHat, nHat), 0.f);
    /* Power per real surface */
    vec3 D = (lightPower * cosTheta) / (PI4*rMag*rMag);
    D = (D + indirectLightPowerPerArea);

    return D;

}


///////////////////////////////////////////////////////////////////
//
//  Functions for clipping
//
///////////////////////////////////////////////////////////////////

void PointEdgeIntersection(vec4 a, vec4 b, Edge &clippingEdge){

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
  vertexPixels[0] = Pixel(10, 5, 0.f, vec4(1.f,0.f,0.f,1.f));
  vertexPixels[1] = Pixel( 5,10, 5.f, vec4(.7f,0.f,0.f,1.f));
  vertexPixels[2] = Pixel(15,15, 6.f, vec4(.9f,0.f,0.f,1.f));

  Pixel v0 = vertexPixels[0];
  Pixel v1 = vertexPixels[2];
  vector<Pixel> result(6);
  Interpolate(v0, v1, result);
  for( uint row=0; row<result.size(); ++row )
  {
    cout << "("
    << result[row].x << ","
    << result[row].y << ","
    << result[row].zinv << ","
    << "("
    << result[row].pos3d.x << ","
    << result[row].pos3d.y << ","
    << result[row].pos3d.z << ","
    << result[row].pos3d.w << ")"
    << endl;
  }
}

/*
void test5(screen *screen){

  memset(screen->depthBuffer, 0, screen->height*screen->width*sizeof(float));
  vec4 pos3d = vec4(1.,1.,1.,1.);

  vector<Pixel> vertexPixels(4);
  vertexPixels[0] = Pixel(1, 1, 0.2f);
  vertexPixels[0].pos3d = pos3d;
  vertexPixels[1] = Pixel(50,50, 0.3f);
  vertexPixels[1].pos3d = pos3d;
  vertexPixels[2] = Pixel(1,50, 2.f);
  vertexPixels[2].pos3d = pos3d;
  vertexPixels[3] = Pixel(50,1, 6.f);
  vertexPixels[3].pos3d = pos3d;

  DrawLineSDL(screen, vertexPixels[0], vertexPixels[1]);
  DrawLineSDL(screen, vertexPixels[2], vertexPixels[3]);
}*/
