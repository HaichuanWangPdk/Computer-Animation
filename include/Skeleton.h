#pragma once

#include "core.h"

#include "Joint.h"

#include "Tokenizer.h"

class Skeleton {
private:
    Joint* root;
    std::vector<Joint*> joints;
    void BuildJointList(Joint* joint);

public:
    Skeleton();
    ~Skeleton();


    bool Load(const char* file);
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);
    void Update();
    glm:: mat4 GetWorldMatrix(int joint) const;

    void DrawDOFGui(); //project 2
    Joint* getRoot();
};