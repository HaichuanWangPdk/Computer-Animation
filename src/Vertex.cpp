#include "Vertex.h"

Vertex::Vertex()
{
    bindPos = glm::vec3(0.0f);
    bindNormal = glm::vec3(0.0f);

    currPos = glm::vec3(0.0f);
    currNormal = glm::vec3(0.0f);

    weights.clear();
}