#ifndef RASTERISER_H
#define RASTERISER_H

#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include <stdint.h>
#include <stdexcept>

using namespace std;
using glm::vec3;
using glm::mat3;
using glm::vec4;
using glm::mat4;
using glm::ivec2;
using glm::vec2;

/* ----------------------------------------------------------------------------*/
/* DATA TYPES                                                                  */   

struct Pixel
{
    int x;
    int y;
    float zinv;

    // default + parameterized constructor
    Pixel(int x=0, int y=0, float zinv=0.0f) 
        : x(x), y(y), zinv(zinv)
    {
    }

    Pixel& operator=(const Pixel& a)
    {
        x=a.x;
        y=a.y;
        zinv=a.zinv;
        return *this;
    }

    Pixel& operator+=(const Pixel& a)
    {
        x+=a.x;
        y+=a.y;
        zinv+=a.zinv;
        return *this;
    }

    Pixel operator+(const Pixel& a) const
    {
        return Pixel(a.x+x, a.y+y, a.zinv+zinv);
    }

    Pixel operator-(const Pixel& a) const
    {
        return Pixel(x-a.x, y-a.y, zinv-a.zinv);
    }

    /*Pixel operator/(const Pixel& a) const
    {
        return Pixel(x/a.x, y/a.y, zinv/a.zinv);
    }*/

    Pixel operator/(const float& a) const
    {
        if(a == 0) throw std::invalid_argument( "division by zero" );
        return Pixel(x/a, y/a, zinv/a);
    }

    bool operator==(const Pixel& a) const
    {
        return (x == a.x && y == a.y && zinv == a.zinv);
    }

    Pixel abs() {
        return Pixel(x>=0?x:-x, y>=0?y:-y, zinv>=0?zinv:-zinv);
    }

};


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update();
void Draw(screen* screen, float* depthBuffer, std::vector<Triangle>& triangles);
void VertexShader( const vec4& v, ivec2& p );
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL( screen* surface, ivec2 a, ivec2 b, vec3 colour);//should we define this in our SLD Auxilarry file
void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices);
void DrawPolygon(screen* screen, const vector<vec4>& vertices, vec3 colour);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawPolygonRows(screen* screen, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 colour);
// Depth buffer versions
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void DrawPolygonRows(screen* screen, float* depthBuffer, const vector<Pixel>& leftPixels, const vector<Pixel>& rightPixels, vec3 colour);
void DrawLineSDL(screen* surface, float* depthBuffer, Pixel a, Pixel b, vec3 colour);
void DrawPolygonDepth(screen* screen, float* depthBuffer, const vector<vec4>& vertices, vec3 colour);
// These are for attempting OpenMP parallel buffers...
void BufferPolygonEdges(uint32_t* buff, const vector<vec4>& vertices);
void DrawLineBuffer(uint32_t* buff, ivec2 a, ivec2 b, vec3 colour);
#endif
