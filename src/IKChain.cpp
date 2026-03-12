#include "IKChain.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

//  Box geometry — two separate VBOs matching Cube.cpp exactly
static const glm::vec3 gPositions[] = {
    // Front
    {-0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f},
    // Back
    { 0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f},
    // Top
    {-0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f, 0.5f}, { 0.5f, 0.5f,-0.5f}, {-0.5f, 0.5f,-0.5f},
    // Bottom
    {-0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f,-0.5f, 0.5f}, {-0.5f,-0.5f, 0.5f},
    // Left
    {-0.5f,-0.5f,-0.5f}, {-0.5f,-0.5f, 0.5f}, {-0.5f, 0.5f, 0.5f}, {-0.5f, 0.5f,-0.5f},
    // Right
    { 0.5f,-0.5f, 0.5f}, { 0.5f,-0.5f,-0.5f}, { 0.5f, 0.5f,-0.5f}, { 0.5f, 0.5f, 0.5f},
};
static const glm::vec3 gNormals[] = {
    { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1}, { 0, 0, 1},
    { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1}, { 0, 0,-1},
    { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0}, { 0, 1, 0},
    { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0}, { 0,-1, 0},
    {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0}, {-1, 0, 0},
    { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0}, { 1, 0, 0},
};
static const unsigned int gIndices[] = {
     0, 1, 2,  0, 2, 3,
     4, 5, 6,  4, 6, 7,
     8, 9,10,  8,10,11,
    12,13,14, 12,14,15,
    16,17,18, 16,18,19,
    20,21,22, 20,22,23,
};
static const int kIdxCount = sizeof(gIndices) / sizeof(unsigned int);

//  Constructor
IKChain::IKChain() {
    const float boneLen[NUM_JOINTS] = { 1.4f, 1.2f, 1.0f, 0.9f, 0.7f };

    for (int i = 0; i < NUM_JOINTS; i++) {
        lengths[i] = boneLen[i];
        joints[i]  = new Joint();

        // Name each joint so DrawDOFGui (if ever used) is readable
        joints[i]->SetName("IK_joint_" + std::to_string(i));
    }

    // Geometry setup mirrors Cube.cpp constructor 
    glGenVertexArrays(1, &boxVAO);
    glGenBuffers(1, &boxVBO_positions);
    glGenBuffers(1, &boxVBO_normals);
    glGenBuffers(1, &boxEBO);

    glBindVertexArray(boxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_positions);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gPositions), gPositions, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ARRAY_BUFFER, boxVBO_normals);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gNormals), gNormals, GL_STATIC_DRAW);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, boxEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gIndices), gIndices, GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);


    ComputeFK();
    std::cout << "[IKChain] Initialized: " << NUM_JOINTS
              << " joints, total reach = " << GetChainLength() << " units\n";
}

IKChain::~IKChain() {
    for (int i = 0; i < NUM_JOINTS; i++)
        delete joints[i];

    // Mirrors Cube destructor
    glDeleteBuffers(1, &boxVBO_positions);
    glDeleteBuffers(1, &boxVBO_normals);
    glDeleteBuffers(1, &boxEBO);
    glDeleteVertexArrays(1, &boxVAO);
}

void IKChain::Reset() {
    for (int i = 0; i < NUM_JOINTS; i++)
        for (int ax = 0; ax < 3; ax++)
            joints[i]->SetDOF(ax, 0.f);
    ComputeFK();
}

float IKChain::GetChainLength() const {
    float total = 0.f;
    for (int i = 0; i < NUM_JOINTS; i++) total += lengths[i];
    return total;
}

//  Forward Kinematics
//
//  Calls Joint::Update(parentMat) exactly as Skeleton does.
//  Joint::Update builds:  worldM = parent * T(offset=0) * Rz * Ry * Rx
//  Since Joint's offset defaults to (0,0,0) — no file loaded — the
//  pivot is purely from the parent transform.  We then advance
//  chainMat by lengths[i] along the joint's local +Y to get the
//  next pivot, matching how the skeleton advances through children.
//
//  parentMats[i] is stored before Update() so we can later extract
//  the world-space rotation axes for the Jacobian (Joint does not
//  store the parent matrix itself — it's just a parameter).
void IKChain::ComputeFK() {
    glm::mat4 chainMat = glm::mat4(1.f);

    for (int i = 0; i < NUM_JOINTS; i++) {
        parentMats[i] = chainMat;                    // cache before rotation applied
        joints[i]->Update(chainMat);                 // worldM = chainMat * Rz*Ry*Rx

        // Advance to tip of this bone along local +Y
        chainMat = joints[i]->GetWorldMatrix()
                 * glm::translate(glm::mat4(1.f), glm::vec3(0.f, lengths[i], 0.f));
    }

    endEffector = glm::vec3(chainMat[3]);
}

//  Inverse Kinematics: Jacobian Transpose
//
//  Joint::Update applies rotations as  T · Rz · Ry · Rx  (slide 20).
//  Per slide 19, the world-space axis for each DOF must account for
//  the rotations that are applied AFTER it in that chain:
//
//    a'_z = W_parent            · [0,0,1]   (nothing follows z)
//    a'_y = W_parent · Rz       · [0,1,0]   (Rz follows y)
//    a'_x = W_parent · Rz · Ry  · [1,0,0]   (Rz and Ry follow x)
//
//  Using just P[ax] (the raw column of W_parent) is only correct for
//  the Z axis; for X and Y it ignores those intermediate rotations.
//
//  Jacobian Transpose update per DOF (slide 4-5):
//    Jcol  = a'_i × (e − r'_i)
//    Δφ_i  = stepSize · Jcol · (g − e)
void IKChain::Solve(const glm::vec3& goal) {
    const float limitRad = glm::radians(maxAngle);

    for (int iter = 0; iter < maxIter; iter++) {
        for (int i = 0; i < NUM_JOINTS; i++) {
            ComputeFK();
            const glm::vec3  e     = endEffector;
            const glm::vec3  error = goal - e;
            if (glm::length(error) < tolerance) break;

            const glm::vec3  pivot = glm::vec3(joints[i]->GetWorldMatrix()[3]);
            const glm::vec3  arm   = e - pivot;
            const glm::mat4& P     = parentMats[i];

            // Build the partial rotation matrices from current DOF values.
            // Order matches Joint::Update: localM = T · Rz · Ry · Rx
            const glm::mat4 Rz = glm::rotate(glm::mat4(1.f), joints[i]->GetDOF(2), glm::vec3(0,0,1));
            const glm::mat4 Ry = glm::rotate(glm::mat4(1.f), joints[i]->GetDOF(1), glm::vec3(0,1,0));

            // World-space axes per slide 19:
            //   a'_x = W_parent · Rz · Ry · X_hat
            //   a'_y = W_parent · Rz       · Y_hat
            //   a'_z = W_parent            · Z_hat
            const glm::vec3 worldAxes[3] = {
                glm::normalize(glm::vec3(P * Rz * Ry * glm::vec4(1,0,0,0))),  // X DOF
                glm::normalize(glm::vec3(P * Rz      * glm::vec4(0,1,0,0))),  // Y DOF
                glm::normalize(glm::vec3(P           * glm::vec4(0,0,1,0))),  // Z DOF
            };

            for (int ax = 0; ax < 3; ax++) {
                float newAngle = joints[i]->GetDOF(ax)
                               + stepSize * glm::dot(glm::cross(worldAxes[ax], arm), error);

                if (doClamp)
                    newAngle = glm::clamp(newAngle, -limitRad, limitRad);

                joints[i]->SetDOF(ax, newAngle);
            }
        }
    }
    ComputeFK();
}

//  SetUniforms — identical to Cube::draw() and Joint::Draw()
void IKChain::SetUniforms(GLuint shader,
                           const glm::mat4& model,
                           const glm::mat4& viewProj,
                           const glm::vec3& color) {
    glUniformMatrix4fv(glGetUniformLocation(shader, "viewProj"),    1, GL_FALSE, (float*)&viewProj);
    glUniformMatrix4fv(glGetUniformLocation(shader, "model"),       1, GL_FALSE, (float*)&model);
    glUniform3fv      (glGetUniformLocation(shader, "DiffuseColor"),1, &color[0]);
}

//  DrawBox — mirrors Cube::draw()
void IKChain::DrawBox(const glm::mat4& model,
                       const glm::vec3& color,
                       const glm::mat4& viewProj,
                       GLuint shader) {
    glUseProgram(shader);
    SetUniforms(shader, model, viewProj, color);
    glBindVertexArray(boxVAO);
    glDrawElements(GL_TRIANGLES, kIdxCount, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

//  DrawLine — mirrors Joint::Draw() temp-VAO pattern
void IKChain::DrawLine(const glm::vec3& p1,
                        const glm::vec3& p2,
                        const glm::vec3& color,
                        const glm::mat4& viewProj,
                        GLuint shader) {
    glm::vec3 verts[2] = { p1, p2 };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0);
    glEnableVertexAttribArray(0);

    glUseProgram(shader);
    SetUniforms(shader, glm::mat4(1.f), viewProj, color);
    glLineWidth(3.5f);
    glDrawArrays(GL_LINES, 0, 2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glUseProgram(0);
}

//  DrawBone
void IKChain::DrawBone(const glm::vec3& p1,
                        const glm::vec3& p2,
                        float            radius,
                        const glm::vec3& color,
                        const glm::mat4& viewProj,
                        GLuint shader) {
    const glm::vec3 delta = p2 - p1;
    const float     bLen  = glm::length(delta);
    if (bLen < 1e-5f) return;

    const glm::vec3 dir  = delta / bLen;
    const glm::vec3 up   = glm::vec3(0.f, 1.f, 0.f);
    const float     cosA = glm::dot(up, dir);

    glm::mat4 R = glm::mat4(1.f);
    if (cosA < -0.9999f) {
        R = glm::rotate(glm::mat4(1.f), glm::pi<float>(), glm::vec3(1,0,0));
    } else if (cosA < 0.9999f) {
        R = glm::rotate(glm::mat4(1.f),
                        glm::acos(glm::clamp(cosA, -1.f, 1.f)),
                        glm::normalize(glm::cross(up, dir)));
    }

    DrawBox(glm::translate(glm::mat4(1.f), p1)
          * R
          * glm::translate(glm::mat4(1.f), glm::vec3(0.f, 0.5f, 0.f))
          * glm::scale(glm::mat4(1.f), glm::vec3(radius, bLen, radius)),
            color, viewProj, shader);
}

//  Draw
void IKChain::Draw(const glm::mat4& viewProj, GLuint shader) {
    const glm::vec3 boneCol (0.85f, 0.62f, 0.15f);
    const glm::vec3 jointCol(0.20f, 0.50f, 0.92f);
    const glm::vec3 rootCol (0.20f, 0.85f, 0.35f);
    const glm::vec3 effCol  (0.95f, 0.20f, 0.20f);

    // Bones: pivot[i] → pivot[i+1], last bone → end effector
    for (int i = 0; i < NUM_JOINTS - 1; i++) {
        DrawBone(glm::vec3(joints[i]->GetWorldMatrix()[3]),
                 glm::vec3(joints[i+1]->GetWorldMatrix()[3]),
                 0.065f, boneCol, viewProj, shader);
    }
    DrawBone(glm::vec3(joints[NUM_JOINTS-1]->GetWorldMatrix()[3]),
             endEffector, 0.065f, boneCol, viewProj, shader);

    // Joint cubes
    for (int i = 0; i < NUM_JOINTS; i++) {
        DrawBox(glm::scale(
                    glm::translate(glm::mat4(1.f),
                                   glm::vec3(joints[i]->GetWorldMatrix()[3])),
                    glm::vec3(i == 0 ? 0.22f : 0.14f)),
                i == 0 ? rootCol : jointCol, viewProj, shader);
    }

    // End effector cube
    DrawBox(glm::scale(glm::translate(glm::mat4(1.f), endEffector), glm::vec3(0.17f)),
            effCol, viewProj, shader);
}

//  DrawGoalf
void IKChain::DrawGoal(const glm::vec3& goal,
                        const glm::mat4& viewProj,
                        GLuint shader) {
    const float arm = 0.45f;
    DrawLine(goal - glm::vec3(arm,0,0), goal + glm::vec3(arm,0,0), {1.f,0.15f,0.15f}, viewProj, shader);
    DrawLine(goal - glm::vec3(0,arm,0), goal + glm::vec3(0,arm,0), {0.15f,1.f,0.15f}, viewProj, shader);
    DrawLine(goal - glm::vec3(0,0,arm), goal + glm::vec3(0,0,arm), {0.15f,0.4f,1.f  }, viewProj, shader);

    DrawBox(glm::scale(glm::translate(glm::mat4(1.f), goal), glm::vec3(0.18f)),
            {1.f, 0.50f, 0.05f}, viewProj, shader);
}
