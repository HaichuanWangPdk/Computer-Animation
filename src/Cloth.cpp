#include "cloth.h"

int Cloth:: GetParticleIndex(int x, int y) const {
    return y * Width + x;
}

void Cloth::InitializeParticles() {
    particles.clear();
    particles.reserve(Width * Height);

    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width; x++) {
            // Center cloth 
            glm::vec3 pos(
                x * Spacing - (Width - 1) * Spacing / 2.0f,  // Center: -5 to +5 for 20¡Á20
                y * Spacing,                          // Start at Y=10
                0.0f
            );
            particles.emplace_back(pos, 1.0f);
        }
    }

    // Fix top row
    int topRowStart = (Height - 1) * Width;
    for (int x = 0; x < Width; x++) {
        particles[topRowStart + x].setFixed(true);
    }
}
void Cloth::InitializeSprings(float springK, float dampingK) {
    springDampers.clear();
    int hzt = Height * (Width - 1);
    int vtc = Width * (Height - 1);
    int dgn = 2 * (Height - 1) * (Width - 1);
    int total = hzt + vtc + dgn;
    springDampers.reserve(total);

    //Horizontal
    for (int y = 0; y < Height; y++) {
        for (int x = 0; x < Width - 1; x++) {
            Particle* p1 = &particles[GetParticleIndex(x, y)];
            Particle* p2 = &particles[GetParticleIndex(x + 1, y)];
            springDampers.emplace_back(springK, dampingK, p1, p2);
        }
    }

    //Vertical
    for (int y = 0; y < Height - 1; y++) {
        for (int x = 0; x < Width; x++) {
            Particle* p1 = &particles[GetParticleIndex(x, y)];
            Particle* p2 = &particles[GetParticleIndex(x, y + 1)];
            springDampers.emplace_back(springK, dampingK, p1, p2);
        }
    }

    //Diagonal 
    for (int y = 0; y < Height - 1; y++) {
        for (int x = 0; x < Width - 1; x++) {
            Particle* p1 = &particles[GetParticleIndex(x, y)];
            Particle* p2 = &particles[GetParticleIndex(x + 1, y + 1)];
            springDampers.emplace_back(springK, dampingK, p1, p2);
        }
    }

    //Diagonal 
    for (int y = 0; y < Height - 1; y++) {
        for (int x = 0; x < Width - 1; x++) {
            Particle* p1 = &particles[GetParticleIndex(x + 1, y)];
            Particle* p2 = &particles[GetParticleIndex(x, y + 1)];
            springDampers.emplace_back(springK, dampingK, p1, p2);
        }
    }
}
void Cloth::InitializeTriangles() {
    triangles.clear();
    int num = 2 * (Height - 1) * (Width - 1);
    triangles.reserve(num);

    for (int y = 0; y < Height - 1; y++) {
        for (int x = 0; x < Width - 1; x++) {
            Particle* p0 = &particles[GetParticleIndex(x, y)];
            Particle* p1 = &particles[GetParticleIndex(x + 1, y)];
            Particle* p2 = &particles[GetParticleIndex(x + 1, y + 1)];
            Particle* p3 = &particles[GetParticleIndex(x, y + 1)];

            triangles.emplace_back(p0, p1, p3);
            triangles.emplace_back(p1, p2, p3);
        }
    }
}

void Cloth::ApplyGravity() {
    for (auto& particle : particles) {
        if (!particle.isFixed()) {
            glm::vec3 gravityForce = Gravity * particle.getMass();
            particle.ApplyForce(gravityForce);
        }
    }
}

void Cloth::ComputeSpringForces() {
    for (auto& spring : springDampers) {
        spring.ComputeNApplyForce();
    }
}

void Cloth::ComputeAerodynamicForces() {
    for (auto& triangle : triangles) {
        triangle.ComputeNApplyForce(Wind);
    }
}

void Cloth::IntegrateParticles(float deltaTime) {
    for (auto& particle : particles) {
        particle.Integrate(deltaTime);
    }
}

void Cloth::HandleGroundCollisions() {
    for (auto& particle : particles) {
        if (particle.isFixed()) continue;

        glm::vec3 pos = particle.getPosition();

        // Check if particle is below ground
        if (pos.y < Ground) {
            glm::vec3 vel = particle.getVelocity();
            glm::vec3 normal = glm::vec3(0.0f, 1.0f, 0.0f);

            // Normal velocity component
            float vn = glm::dot(vel, normal);

            // Only respond if moving into ground
            if (vn < 0.0f) {
                float mass = particle.getMass();

                // Normal impulse: j = -(1 + ¦Å) ¡Á m ¡Á vn
                float jn = -(1.0f + Elasticity) * mass * vn;
                glm::vec3 impulseN = jn * normal;

                // Tangential velocity
                glm::vec3 vt = vel - vn * normal;
                float vtMag = glm::length(vt);

                // Friction impulse
                glm::vec3 impulseT = glm::vec3(0.0f);
                if (vtMag > 0.0001f) {
                    glm::vec3 tangent = vt / vtMag;

                    // Friction magnitude (capped)
                    float jt = Friction * jn;
                    if (jt > mass * vtMag) {
                        jt = mass * vtMag;
                    }

                    impulseT = -jt * tangent;
                }

                // Apply impulse
                particle.ApplyImpulse(impulseN + impulseT);
            }

            // Position correction
            particle.setPosition(glm::vec3(pos.x, Ground + 0.001f, pos.z));
        }
    }
}

void Cloth::ComputeSmoothNormals() {
    //Zero out all particle normals
    for (auto& particle : particles) {
        particle.ZeroNormal();
    }

    //Add triangle normals to vertices
    for (auto& triangle : triangles) {
        triangle.AddNormals();
    }

    // Normalize all particle normals
    for (auto& particle : particles) {
        particle.NormalizeNormal();
    }
}



Cloth::Cloth() {
    Width = 20;
    Height = 20;
    Spacing = 1.0f;
    Wind = glm::vec3(0.0);
    Gravity = glm::vec3(0.0f, -9.8f, 0.0f);
    Ground = 0.0f;
    Elasticity = 0.05f;
    Friction = 0.6f;

    InitializeParticles();
    InitializeSprings(1000.0f, 10.0f);
    InitializeTriangles();
}

Cloth::Cloth(int width, int height, float spacing, float springK, float dampingK) {
    Width = width;
    Height = height;
    Spacing = spacing;
    Wind = glm::vec3(0.0f);
    Gravity = glm::vec3(0.0f, -9.8f, 0.0f);
    Ground = 0.0f;
    Elasticity = 0.05f;
    Friction = 0.6f;

    InitializeParticles();
    InitializeSprings(springK, dampingK);
    InitializeTriangles();
}

int Cloth::getWidth() const {
    return Width;
}

int Cloth::getHeight() const {
    return Height;
}

float Cloth::getSpacing() const {
    return Spacing;
}

glm::vec3 Cloth::getWind() const {
    return Wind;
}

glm::vec3 Cloth::getGravity() const {
    return Gravity;
}

float Cloth::getGround() const {
    return Ground;
}

float Cloth::getElasticity() const {
    return Elasticity;
}

float Cloth::getFriction() const {
    return Friction;
}

void Cloth::setWind(const glm::vec3& wind) {
    Wind = wind;
}

void Cloth::setGravity(const glm::vec3& gravity) {
    Gravity = gravity;
}

void Cloth::setGround(float ground) {
    Ground = ground;
}

void Cloth::setElasticity(float elasticity) {
    Elasticity = elasticity;
}

void Cloth::setFriction(float friction) {
    Friction = friction;
}

void Cloth::Update(float deltaTime) {
    this->ApplyGravity();
    this->ComputeSpringForces();
    this->ComputeAerodynamicForces();
    this->IntegrateParticles(deltaTime);
    this->HandleGroundCollisions();
    this->ComputeSmoothNormals();
}

void Cloth::Draw(const glm::mat4& viewProjMtx, GLuint shader) {
    // Use the shader program (same as Joint)
    glUseProgram(shader);

    // Get uniform locations (same as Joint)
    GLuint viewProjLoc = glGetUniformLocation(shader, "viewProj");
    GLuint modelLoc = glGetUniformLocation(shader, "model");

    // Set the matrix uniforms (same as Joint)
    if (viewProjLoc != -1) {
        glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &viewProjMtx[0][0]);
    }

    // Model matrix is identity for cloth (in world space)
    glm::mat4 model = glm::mat4(1.0f);
    if (modelLoc != -1) {
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    }

    // Set color (use DiffuseColor like Joint, not "color")
    GLuint colorLoc = glGetUniformLocation(shader, "DiffuseColor");
    if (colorLoc != -1) {
        glm::vec3 clothColor(0.8f, 0.6f, 0.4f);  // Beige cloth
        glUniform3fv(colorLoc, 1, &clothColor[0]);
    }

    // Collect vertex data (positions and normals)
    std::vector<glm::vec3> positions;
    std::vector<glm::vec3> normals;
    positions.reserve(particles.size());
    normals.reserve(particles.size());

    for (const auto& p : particles) {
        positions.push_back(p.getPosition());
        normals.push_back(p.getNormal());
    }

    // Build index buffer
    std::vector<unsigned int> indices;
    indices.reserve((Height - 1) * (Width - 1) * 6);

    for (int y = 0; y < Height - 1; y++) {
        for (int x = 0; x < Width - 1; x++) {
            int i0 = GetParticleIndex(x, y);
            int i1 = GetParticleIndex(x + 1, y);
            int i2 = GetParticleIndex(x, y + 1);
            int i3 = GetParticleIndex(x + 1, y + 1);

            // Triangle 1
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            // Triangle 2
            indices.push_back(i1);
            indices.push_back(i3);
            indices.push_back(i2);
        }
    }

    // Create and bind VAO/VBO (same pattern as Joint)
    GLuint VAO, VBO_pos, VBO_norm, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO_pos);
    glGenBuffers(1, &VBO_norm);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);

    // Bind and set position buffer (location 0)
    glBindBuffer(GL_ARRAY_BUFFER, VBO_pos);
    glBufferData(GL_ARRAY_BUFFER, positions.size() * sizeof(glm::vec3), positions.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(0);

    // Bind and set normal buffer (location 1)
    glBindBuffer(GL_ARRAY_BUFFER, VBO_norm);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(glm::vec3), (void*)0);
    glEnableVertexAttribArray(1);

    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Draw the cloth triangles
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);

    // Cleanup (same as Joint - delete immediately)
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    glDeleteBuffers(1, &VBO_pos);
    glDeleteBuffers(1, &VBO_norm);
    glDeleteBuffers(1, &EBO);
    glDeleteVertexArrays(1, &VAO);
}

void Cloth::ReleaseAllFixed() {
    for (auto& particle : particles) {
        particle.setFixed(false);
    }
}

void Cloth::TranslateFixedRow(const glm::vec3& delta) {
    for (int x = 0; x < Width; x++) {
        int idx = GetParticleIndex(x, Height - 1);
        Particle& p = particles[idx];
        if (p.isFixed()) {
            p.setPosition(p.getPosition() + delta);
        }
    }
}

void Cloth::RotateFixedRow(float angleDegrees, const glm::vec3& axis) {
    // Compute center of top row
    glm::vec3 center(0.0f);
    for (int x = 0; x < Width; x++) {
        int idx = GetParticleIndex(x, Height - 1);
        center += particles[idx].getPosition();
    }
    center /= static_cast<float>(Width);

    float angleRad = glm::radians(angleDegrees);
    glm::vec3 u = glm::normalize(axis);
    float cosA = cos(angleRad);
    float sinA = sin(angleRad);

    for (int x = 0; x < Width; x++) {
        int idx = GetParticleIndex(x, Height - 1);
        Particle& p = particles[idx];
        if (p.isFixed()) {
            glm::vec3 v = p.getPosition() - center;

            // Rodrigues' rotation formula
            glm::vec3 rotated = v * cosA
                + glm::cross(u, v) * sinA
                + u * glm::dot(u, v) * (1.0f - cosA);

            p.setPosition(center + rotated);
        }
    }
}