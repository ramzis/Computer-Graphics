#ifndef RASTERISER_H
#define RASTERISER_H

#include <iostream>
#include <glm/glm.hpp>
#include <SDL.h>
#include "SDLauxiliary.h"
#include "TestModelH.h"
#include "Camera.h"
#include <stdint.h>
#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>

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
    vec4 pos3d;

    // default + parameterized constructor
    Pixel(int x=0, int y=0, float zinv=0.0f, vec4 pos3d=vec4(0.f,0.f,0.f,1.f))
        : x(x), y(y), zinv(zinv), pos3d(pos3d)
    {
    }

    Pixel& operator=(const Pixel& a)
    {
        x=a.x;
        y=a.y;
        zinv=a.zinv;
        pos3d=a.pos3d;
        return *this;
    }

    Pixel& operator+=(const Pixel& a)
    {
        x+=a.x;
        y+=a.y;
        zinv+=a.zinv;
        pos3d+=a.pos3d;
        return *this;
    }

    Pixel operator+(const Pixel& a) const
    {
        return Pixel(a.x+x, a.y+y, a.zinv+zinv, a.pos3d+pos3d);
    }

    Pixel operator-(const Pixel& a) const
    {
        return Pixel(x-a.x, y-a.y, zinv-a.zinv, pos3d-a.pos3d);
    }

    /*Pixel operator/(const Pixel& a) const
    {
        return Pixel(x/a.x, y/a.y, zinv/a.zinv);
    }*/

    Pixel operator/(const float& a) const
    {
        if(a == 0) throw std::invalid_argument( "division by zero" );
        return Pixel(x/a, y/a, zinv/a, pos3d/a);
    }

    bool operator==(const Pixel& a) const
    {
        return (x == a.x && y == a.y && zinv == a.zinv && pos3d == a.pos3d);
    }

    Pixel abs() {
        return Pixel(x>=0?x:-x, y>=0?y:-y, zinv>=0?zinv:-zinv);
    }


};

std::ostream& operator<<(std::ostream &o, const Pixel &a)
{
    o << "(" << a.x << "," << a.y << "," << a.zinv << ")";
    return o;
}

std::ostream& operator<<(std::ostream &o, const vec3 &a)
{
    o << "(" << a.x << "," << a.y << "," << a.z << ")";
    return o;
}

std::ostream& operator<<(std::ostream &o, const vec4 &a)
{
    o << "(" << a.x << "," << a.y << "," << a.z << "," << a.w  << ")";
    return o;
}

struct Vertex {
    vec4 pos;

    Vertex(vec4 pos=vec4(0.f,0.f,0.f,1.f))
        : pos(pos)
    {
    }
};

struct Edge {
  vec4 start;
  vec4 end;

};


/* ----------------------------------------------------------------------------*/
/* FUNCTIONS                                                                   */

void Update(Camera &camera);
void UpdateCamera(Camera &camera, const Uint8* keystate, float deltaTime);
void Draw(screen* screen, Camera &camera, std::vector<Triangle>& triangles);
void VertexShader( const vec4& v, ivec2& p );
void Interpolate(ivec2 a, ivec2 b, vector<ivec2>& result);
void DrawLineSDL( screen* surface, ivec2 a, ivec2 b, vec3 colour);//should we define this in our SLD Auxilarry file
void DrawPolygonEdges(screen* screen, const vector<vec4>& vertices);
void DrawPolygon(screen* screen, const vector<vec4>& vertices, vec3 colour);
void ComputePolygonRows(const vector<ivec2>& vertexPixels, vector<ivec2>& leftPixels, vector<ivec2>& rightPixels);
void DrawPolygonRows(screen* screen, const vector<ivec2>& leftPixels, const vector<ivec2>& rightPixels, vec3 colour);
// Depth buffer versions
void Interpolate(Pixel a, Pixel b, vector<Pixel>& result);
void DrawPolygonDepth(screen* screen, Camera &camera, const Triangle &t);
void ComputePolygonRows(const vector<Pixel>& vertexPixels, vector<Pixel>& leftPixels, vector<Pixel>& rightPixels);
void VertexShader(Camera &camera, const Vertex& v, Pixel& p );
void PixelShader(screen* screen, const Pixel& p, const vec4 &normal, const vec3 &colour, const vec3 &reflectance);
void DrawLineSDL(screen* screen, Pixel& a, Pixel& b, const vec4 &normal, const vec3 &colour, const vec3 &reflectance);
vec3 DirectLight(const Pixel &p, const vec4 &normal, const vec3 &reflectance);
// These are for attempting OpenMP parallel buffers...
void BufferPolygonEdges(uint32_t* buff, const vector<vec4>& vertices);
void DrawLineBuffer(uint32_t* buff, ivec2 a, ivec2 b, vec3 colour);
//for clipping
void PointEdgeIntersection(vec4 a, vec4 b, Edge &clippingEdge);
// Test functions
void test1();
void test2();
void test3();
void test4();
#endif
