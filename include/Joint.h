#pragma once

#include <vector>

#include "core.h"

#include "Tokenizer.h"

#include <string>

class Joint {
private:
    struct DOF {
        glm::vec3 value = glm::vec3(0, 0, 0);
        glm::vec3 rotMin = glm::vec3(-100000, -100000, -100000);
        glm::vec3 rotMax = glm::vec3(100000, 100000, 100000);
    };

    glm::mat4 localM;
    glm::mat4 worldM;
    glm::vec3 offset;
    glm::vec3 boxmin;
    glm::vec3 boxmax;
    struct DOF dof;
    Joint* parent;
    std::vector<Joint*> children;

    int index; //added for project 2
    std::string name;

    glm::vec3 animOffset;//added for project 3 root translation


public:
    Joint();
    ~Joint();

    
    void AddChild(Joint* child);
    bool Load(Tokenizer& t);
    void Draw(const glm::mat4& viewProjMtx, GLuint shader);
    void Update(const glm::mat4& parent);

    //added for project 2
    void SetIndex(int i) { index = i; }
    int GetIndex() const { return index; }

    const glm::mat4& GetWorldMatrix() const { return worldM; }

    const std::vector<Joint*>& GetChildren() const { return children; }

    float GetDOF(int axis) const;
    void SetDOF(int axis, float value);

    float GetDOFMin(int axis) const;
    float GetDOFMax(int axis) const;
    const std::string& GetName() const { return name; }
    void SetName(const std::string& n) { name = n; }

    void DrawDOFGui();

    void SetAnimationOffset(const glm::vec3& animOff);//added for project 3 root translation
};