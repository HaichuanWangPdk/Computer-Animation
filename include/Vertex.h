#pragma once

#include <vector>

#include "core.h"

struct SkinWeight {
    int joint;      // joint index
    float weight;   // weight value
};

class Vertex {
public:
    glm::vec3 bindPos;        // original position (binding space)
    glm::vec3 bindNormal;     // original normal
    glm::vec3 currPos;        // deformed position (world space)
    glm::vec3 currNormal;     // deformed normal

    std::vector<SkinWeight> weights;

    Vertex();
};
