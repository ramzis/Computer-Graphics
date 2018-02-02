#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <vector>

class Camera
{
public:
	glm::vec4 pos;

	Camera()
	: pos(0,0,0,1)
	{
	}

};

#endif
