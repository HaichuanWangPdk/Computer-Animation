#include "Joint.h"

#include "Tokenizer.h"

#include "core.h"

#include "imgui.h"      

Joint::Joint() {
    localM = glm::mat4(1.0f);  
    worldM = glm::mat4(1.0f); 

    // Initialize vectors to zero
    offset = glm::vec3(0.0f, 0.0f, 0.0f);
    boxmin = glm::vec3(-0.1f, -0.1f, -0.1f);
    boxmax = glm::vec3(0.1f, 0.1f, 0.1f);

    parent = nullptr;
    index = -1; //project 2

    animOffset = glm::vec3(0.0f, 0.0f, 0.0f);  // Initialize for project 3
}

Joint::~Joint() {
    for (Joint* child : children) {
        delete child;
    }
    parent = nullptr;
}

void Joint::AddChild(Joint* child) {
    children.push_back(child);
}

bool Joint::Load(Tokenizer& t) {
    if (!t.FindToken("{")) {
        return false;
    }

    // Now parse everything inside { }
    while (1) {
        char temp[256];
        t.GetToken(temp);  

        if (strcmp(temp, "offset") == 0) {
            offset.x = t.GetFloat();
            offset.y = t.GetFloat();
            offset.z = t.GetFloat();
        }
        else if (strcmp(temp, "boxmin") == 0) {
            boxmin.x = t.GetFloat();
            boxmin.y = t.GetFloat();
            boxmin.z = t.GetFloat();
        }
        else if (strcmp(temp, "boxmax") == 0) {
            boxmax.x = t.GetFloat();
            boxmax.y = t.GetFloat();
            boxmax.z = t.GetFloat();
        }
        else if (strcmp(temp, "rotxlimit") == 0) {
            dof.rotMin.x = t.GetFloat();
            dof.rotMax.x = t.GetFloat();
        }
        else if (strcmp(temp, "rotylimit") == 0) {
            dof.rotMin.y = t.GetFloat();
            dof.rotMax.y = t.GetFloat();
        }
        else if (strcmp(temp, "rotzlimit") == 0) {
            dof.rotMin.z = t.GetFloat();
            dof.rotMax.z = t.GetFloat();
        }
        else if (strcmp(temp, "pose") == 0) {
            dof.value.x = t.GetFloat();
            dof.value.y = t.GetFloat();
            dof.value.z = t.GetFloat();
        }
        else if (strcmp(temp, "balljoint") == 0) {
            char jointName[256];
            t.GetToken(jointName);

            Joint* jnt = new Joint();
            jnt->name = jointName;
            jnt->Load(t);
            AddChild(jnt);
        }
        else if (strcmp(temp, "}") == 0) {
            return true;
        }
        else {
            t.SkipLine();
        }
    }

    return true;
}

void Joint::Update(const glm::mat4& parent) {
    localM = glm::mat4(1.0f);
    glm::vec3 totalOffset = offset + animOffset;//added for project 3
    localM = glm::translate(localM, totalOffset);
    
    localM = glm::rotate(localM, dof.value.z, glm::vec3(0, 0, 1));
    localM = glm::rotate(localM, dof.value.y, glm::vec3(0, 1, 0));
    localM = glm::rotate(localM, dof.value.x, glm::vec3(1, 0, 0));
    
    

    worldM = parent * localM;// Compute world matrix W

    for (Joint* child: children) {
        child->Update(worldM);// Recursively call Update() on children
    }
}

void Joint::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
    // Use the shader program
    glUseProgram(shader);

    // Get uniform locations for viewProj and model matrices (matching Cube.cpp)
    GLuint viewProjLoc = glGetUniformLocation(shader, "viewProj");
    GLuint modelLoc = glGetUniformLocation(shader, "model");

    // Set the matrix uniforms (same as Cube.cpp)
    if (viewProjLoc != -1) {
        glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &viewProjMtx[0][0]);
    }
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &worldM[0][0]);
    }

    // Also set a color for the skeleton (optional but recommended)
    GLuint colorLoc = glGetUniformLocation(shader, "DiffuseColor");
    if (colorLoc != -1) {
        glm::vec3 color(1.0f, 1.0f, 1.0f);  // Green color for skeleton
        glUniform3fv(colorLoc, 1, &color[0]);
    }

    // Generate vertices for the wireframe box
    glm::vec3 vertices[8] = {
        glm::vec3(boxmin.x, boxmin.y, boxmin.z),
        glm::vec3(boxmax.x, boxmin.y, boxmin.z),
        glm::vec3(boxmax.x, boxmax.y, boxmin.z),
        glm::vec3(boxmin.x, boxmax.y, boxmin.z),
        glm::vec3(boxmin.x, boxmin.y, boxmax.z),
        glm::vec3(boxmax.x, boxmin.y, boxmax.z),
        glm::vec3(boxmax.x, boxmax.y, boxmax.z),
        glm::vec3(boxmin.x, boxmax.y, boxmax.z)
    };

    // Indices for the 12 edges of the box
    GLuint indices[24] = {
        0,1, 1,2, 2,3, 3,0,  // back face
        4,5, 5,6, 6,7, 7,4,  // front face
        0,4, 1,5, 2,6, 3,7   // connecting edges
    };

    // Create and bind VAO/VBO
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Bind and set vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Set vertex attribute pointers
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Draw the box edges
    glDrawElements(GL_LINES, 24, GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);

    // Draw children recursively
    for (Joint* child : children) {
        child->Draw(viewProjMtx, shader);
    }
}

float Joint::GetDOF(int axis) const {
    return dof.value[axis];
}

float Joint::GetDOFMin(int axis) const {
    return dof.rotMin[axis];
}

float Joint::GetDOFMax(int axis) const {
    return dof.rotMax[axis];
}

void Joint::SetDOF(int axis, float value) {
    // Clamp to limits
    value = glm::clamp(value, dof.rotMin[axis], dof.rotMax[axis]);
    dof.value[axis] = value;
}

void Joint::DrawDOFGui() {
    if (ImGui::TreeNode(name.c_str())) {

        ImGui::SliderFloat("Rot X",
            &dof.value.x,
            dof.rotMin.x,
            dof.rotMax.x);

        ImGui::SliderFloat("Rot Y",
            &dof.value.y,
            dof.rotMin.y,
            dof.rotMax.y);

        ImGui::SliderFloat("Rot Z",
            &dof.value.z,
            dof.rotMin.z,
            dof.rotMax.z);

        for (Joint* child : children)
            child->DrawDOFGui();

        ImGui::TreePop();
    }
}

void Joint::SetAnimationOffset(const glm::vec3& animOff) {
    animOffset = animOff;
}