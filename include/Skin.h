#pragma once

#include <vector>

#include "core.h"

#include "Tokenizer.h"

#include "Skeleton.h"

#include "Vertex.h"

#include "Triangle.h"

class Skin{
private:
		std::vector<Vertex> vertices;
        std::vector<Triangle> triangles;

        // Binding matrices (one per joint)
        std::vector<glm::mat4> bindMatrices;

        GLuint VAO, VBO, EBO;

public:
    Skin();
    ~Skin();

    bool Load(const char* filename);

    void Update(Skeleton* skeleton);   // performs skinning
    void Draw(const glm::mat4& viewProj, GLuint shader);

    int GetNumVertices() const;
    int GetNumTriangles() const;
};