#include "render/Model.hpp"
#include <GL/glew.h>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>


Model::Geometry::Geometry()
	: flags(0)
{
	
}

Model::Geometry::~Geometry()
{

}

ModelFrame::ModelFrame(unsigned int index, ModelFrame* parent, glm::mat3 dR, glm::vec3 dT)
 : index(index), defaultRotation(dR), defaultTranslation(dT), parentFrame(parent)
{
	if(parent != nullptr) {
		parent->childs.push_back(this);
	}
	reset();
}

void ModelFrame::reset()
{
	matrix = glm::translate(glm::mat4(), defaultTranslation) * glm::mat4(defaultRotation);
}

void ModelFrame::addGeometry(size_t idx)
{
	geometries.push_back(idx);
}


Model::~Model()
{
	for(auto mf : frames) {
		delete mf;
	}
}

float Model::getBoundingRadius() const
{
	float mindist = std::numeric_limits<float>::min();
	for (size_t g = 0; g < geometries.size(); g++)
	{
		RW::BSGeometryBounds& bounds = geometries[g]->geometryBounds;
		mindist = std::max(mindist, glm::length(bounds.center) + bounds.radius);
	}
	return mindist;
}
