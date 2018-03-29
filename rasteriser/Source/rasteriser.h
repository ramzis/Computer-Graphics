#ifndef RASTERISER_H
#define RASTERISER_H

#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::ivec2;
using glm::vec2;


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen, std::vector<Triangle>& triangles);
void VertexShader( const vec4& v, ivec2& p );
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL( screen* surface, ivec2 a, ivec2 b, vec3 colour);//should we define this in our SLD Auxilarry file
void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices);
void DrawPolygon(screen* screen, const vector<vec4>& vertices, vec3 colour);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawPolygonRows(screen* screen, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 colour);
// These are for attempting OpenMP parallel buffers...
void BufferPolygonEdges(uint32_t* buff, const vector<vec4>& vertices);
void DrawLineBuffer(uint32_t* buff, ivec2 a, ivec2 b, vec3 colour);
#endif
