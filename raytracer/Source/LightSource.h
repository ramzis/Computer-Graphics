#ifndef LIGHTSOURCE_H
#define LIGHTSOURCE_H

using glm::vec4;
using glm::vec3;

class LightSource {

public:
	/*light position */
	glm::vec4 pos;
	/* Light colour */
	glm::vec3 color;

	LightSource(glm::vec4 pos, glm::vec3 color)
	: pos(pos), color(color)
	{
	}

};

#endif

