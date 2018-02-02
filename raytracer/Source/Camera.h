#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	/* Camera position */
	glm::vec4 pos;
	/* Focal length */
	float f;

	Camera(glm::vec4 pos, float f)
	: pos(pos), f(f)
	{
	}

};

#endif
