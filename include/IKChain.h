#pragma once
#include "core.h"
#include "Joint.h"   // reuse Joint directly — no inner struct needed

// ============================================================
//  IKChain  —  5-link 3D chain solved with Jacobian Transpose.
//
//  Each link is a real Joint (3-DOF ball joint).
//  Joint::Update(parentMat) handles FK exactly as in Skeleton.
//  Joint::GetDOF / SetDOF access the rotation angles.
//  Joint::GetWorldMatrix() gives us worldM and the pivot.
//
//  parentMats[] is the only IK-specific parallel array: Joint
//  takes parentMat as a parameter to Update() but does not store
//  it, so we cache it here for the Jacobian axis extraction.
//  lengths[] stores the bone length along local +Y between pivots.
// ============================================================
class IKChain {
public:
    static const int NUM_JOINTS = 5;

    IKChain();
    ~IKChain();

    // ---------- IK ----------
    void Solve(const glm::vec3& goal);
    void Reset();

    // ---------- Query ----------
    glm::vec3 GetEndEffector() const { return endEffector; }
    float     GetChainLength()  const;

    // ---------- Rendering ----------
    void Draw    (const glm::mat4& viewProj, GLuint shader);
    void DrawGoal(const glm::vec3& goal, const glm::mat4& viewProj, GLuint shader);

    // ---------- Solver params (tweakable via ImGui) ----------
    float stepSize  = 0.12f;
    int   maxIter   = 25;
    float tolerance = 0.005f;
    bool  doClamp   = true;
    float maxAngle  = 150.0f;   // degrees, ± per axis

private:
    Joint*    joints[NUM_JOINTS];       // the chain links
    float     lengths[NUM_JOINTS];      // bone length along +Y per link
    glm::mat4 parentMats[NUM_JOINTS];   // cached for Jacobian axis extraction

    glm::vec3 endEffector = glm::vec3(0.f);

    void ComputeFK();

    // ---- Rendering (same uniform names as Cube::draw / Joint::Draw) ----
    void SetUniforms(GLuint shader, const glm::mat4& model,
                     const glm::mat4& viewProj, const glm::vec3& color);

    void DrawBox (const glm::mat4& model, const glm::vec3& color,
                  const glm::mat4& viewProj, GLuint shader);
    void DrawLine(const glm::vec3& p1, const glm::vec3& p2,
                  const glm::vec3& color, const glm::mat4& viewProj, GLuint shader);
    void DrawBone(const glm::vec3& p1, const glm::vec3& p2, float radius,
                  const glm::vec3& color, const glm::mat4& viewProj, GLuint shader);

    // Two separate VBOs — matches Cube.cpp layout, set up once in constructor
    GLuint boxVAO           = 0;
    GLuint boxVBO_positions = 0;
    GLuint boxVBO_normals   = 0;
    GLuint boxEBO           = 0;
};
