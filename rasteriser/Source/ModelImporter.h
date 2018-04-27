#ifndef MODELIMPORTER_H
#define MODELIMPORTER_H

#include <vector>
#include "TestModelH.h"


class Importer {
	public:
		char *fileName;
		std::vector<Triangle> triangles;

		void fromFile(char *file){
			fileName = file;
			
			//TODO: read file


		}

};



#endif