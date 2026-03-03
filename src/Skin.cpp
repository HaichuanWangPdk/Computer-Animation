#include "Skin.h"

#include <iostream>

Skin::Skin() {
    vertices.clear();
    triangles.clear();
    bindMatrices.clear();

    VAO = 0;
    VBO = 0;
    EBO = 0;
}

Skin::~Skin() {
 
}

bool Skin::Load(const char* filename) {
    Tokenizer token;

    if (!token.Open(filename)) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return false;
    }

    std::cout << "Loading skin: " << filename << std::endl;

    char temp[256];

    while (token.GetToken(temp)) {
        // Check for empty token to prevent infinite loop
        if (strlen(temp) == 0) {
            break;
        }

        std::cout << "Token: " << temp << std::endl;  // DEBUG - can remove later

        if (strcmp(temp, "positions") == 0) {
            int numVerts = token.GetInt();

            if (!token.FindToken("{")) {
                return false;
            }

            vertices.resize(numVerts);

            for (int i = 0; i < numVerts; i++) {
                float x = token.GetFloat();
                float y = token.GetFloat();
                float z = token.GetFloat();
                vertices[i].bindPos = glm::vec3(x, y, z);
                vertices[i].currPos = vertices[i].bindPos;
            }

            token.FindToken("}");

        }
        else if (strcmp(temp, "normals") == 0) {
            int numVerts = token.GetInt();

            if (!token.FindToken("{")) {
                return false;
            }

            for (int i = 0; i < numVerts; i++) {
                float x = token.GetFloat();
                float y = token.GetFloat();
                float z = token.GetFloat();
                vertices[i].bindNormal = glm::vec3(x, y, z);
                vertices[i].currNormal = vertices[i].bindNormal;
            }

            token.FindToken("}");

        }
        else if (strcmp(temp, "skinweights") == 0) {
            int numVerts = token.GetInt();

            if (!token.FindToken("{")) {
                return false;
            }

            for (int i = 0; i < numVerts; i++) {
                int numBinds = token.GetInt();
                vertices[i].weights.resize(numBinds);

                for (int j = 0; j < numBinds; j++) {
                    vertices[i].weights[j].joint = token.GetInt();
                    vertices[i].weights[j].weight = token.GetFloat();
                }
            }

            token.FindToken("}");

        }
        else if (strcmp(temp, "triangles") == 0) {
            int numTris = token.GetInt();

            if (!token.FindToken("{")) {
                return false;
            }

            triangles.resize(numTris);
            for (int i = 0; i < numTris; i++) {
                int a = token.GetInt();
                int b = token.GetInt();
                int c = token.GetInt();
                triangles[i] = Triangle(a, b, c);
            }

            token.FindToken("}");

        }
        else if (strcmp(temp, "bindings") == 0) {
            int numJoints = token.GetInt();
            bindMatrices.resize(numJoints);

            if (!token.FindToken("{")) {
                return false;
            }

            for (int i = 0; i < numJoints; i++) {
                if (!token.FindToken("matrix")) {
                    return false;
                }
                if (!token.FindToken("{")) {
                    return false;
                }

                // Read 4 rows of 3 values each from the file
                float mat[4][4];
                for (int row = 0; row < 4; row++) {
                    mat[row][0] = token.GetFloat();  // x
                    mat[row][1] = token.GetFloat();  // y
                    mat[row][2] = token.GetFloat();  // z
                    mat[row][3] = (row == 3) ? 1.0f : 0.0f;  // w component
                }

                // The file gives us the BIND POSE matrix (where joint was during binding)
                // Build the matrix in GLM format (column-major)
                // GLM constructor takes columns, so we pass rows as columns (transpose)
                glm::mat4 bindPose(
                    mat[0][0], mat[0][1], mat[0][2], mat[0][3],  // column 0 = row 0
                    mat[1][0], mat[1][1], mat[1][2], mat[1][3],  // column 1 = row 1
                    mat[2][0], mat[2][1], mat[2][2], mat[2][3],  // column 2 = row 2
                    mat[3][0], mat[3][1], mat[3][2], mat[3][3]   // column 3 = row 3 (translation)
                );

                // For skinning, we need the INVERSE of the bind pose matrix
                bindMatrices[i] = glm::inverse(bindPose);

                token.FindToken("}");  // closing brace of matrix
            }

            token.FindToken("}");  // closing brace of bindings section
        }
        else {
            token.SkipLine();
        }
    }

    std::cout << "Finished loading skin" << std::endl;
    std::cout << "Vertices: " << vertices.size() << std::endl;
    std::cout << "Triangles: " << triangles.size() << std::endl;
    std::cout << "Bind matrices: " << bindMatrices.size() << std::endl;

    token.Close();
    return true;
}

void Skin::Update(Skeleton* skeleton) {
    if (skeleton) {
        // DEBUG: Print first vertex info
        static bool printed = false;
        if (!printed && vertices.size() > 0) {
            std::cout << "\n=== DEBUG: First Vertex Skinning ===" << std::endl;
            std::cout << "Bind Position: " << vertices[0].bindPos.x << ", "
                << vertices[0].bindPos.y << ", " << vertices[0].bindPos.z << std::endl;
            std::cout << "Num weights: " << vertices[0].weights.size() << std::endl;

            for (size_t j = 0; j < vertices[0].weights.size(); j++) {
                int jointIdx = vertices[0].weights[j].joint;
                float weight = vertices[0].weights[j].weight;

                std::cout << "\nWeight " << j << ": joint=" << jointIdx << ", weight=" << weight << std::endl;

                glm::mat4 worldMatrix = skeleton->GetWorldMatrix(jointIdx);
                std::cout << "World Matrix:" << std::endl;
                for (int row = 0; row < 4; row++) {
                    std::cout << "  " << worldMatrix[0][row] << " " << worldMatrix[1][row]
                        << " " << worldMatrix[2][row] << " " << worldMatrix[3][row] << std::endl;
                }

                std::cout << "Bind Matrix:" << std::endl;
                for (int row = 0; row < 4; row++) {
                    std::cout << "  " << bindMatrices[jointIdx][0][row] << " " << bindMatrices[jointIdx][1][row]
                        << " " << bindMatrices[jointIdx][2][row] << " " << bindMatrices[jointIdx][3][row] << std::endl;
                }
            }
            printed = true;
        }

        // Normal skinning code
        for (size_t i = 0; i < vertices.size(); i++) {
            glm::vec4 skinnedPos(0.0f);
            glm::vec3 skinnedNormal(0.0f);

            for (const SkinWeight& w : vertices[i].weights) {
                int jointIdx = w.joint;
                float weight = w.weight;

                glm::mat4 worldMatrix = skeleton->GetWorldMatrix(jointIdx);
                glm::mat4 skinMtx = worldMatrix * bindMatrices[jointIdx];

                skinnedPos += weight * (skinMtx * glm::vec4(vertices[i].bindPos, 1.0f));
                skinnedNormal += weight * (glm::mat3(skinMtx) * vertices[i].bindNormal);
            }

            vertices[i].currPos = glm::vec3(skinnedPos);
            vertices[i].currNormal = glm::normalize(skinnedNormal);
        }
    }
    else {
        // No skeleton - just copy bind positions
        for (size_t i = 0; i < vertices.size(); i++) {
            vertices[i].currPos = vertices[i].bindPos;
            vertices[i].currNormal = vertices[i].bindNormal;
        }
    }
}

int Skin::GetNumVertices() const {
    return static_cast<int>(vertices.size());
}

int Skin::GetNumTriangles() const {
    return static_cast<int>(triangles.size());
}
void Skin::Draw(const glm::mat4& viewProj, GLuint shader) {
    // Nothing to draw
    if (vertices.empty() || triangles.empty()) return;

    // Use shader and set uniforms
    glUseProgram(shader);
    GLuint viewProjLoc = glGetUniformLocation(shader, "viewProj");
    if (viewProjLoc != -1) glUniformMatrix4fv(viewProjLoc, 1, GL_FALSE, &viewProj[0][0]);

    GLuint modelLoc = glGetUniformLocation(shader, "model");
    if (modelLoc != -1) {
        glm::mat4 model(1.0f);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, &model[0][0]);
    }

    GLuint colorLoc = glGetUniformLocation(shader, "DiffuseColor");
    if (colorLoc != -1) {
        glm::vec3 color(0.8f, 0.8f, 0.8f);
        glUniform3fv(colorLoc, 1, &color[0]);
    }

    // Build interleaved vertex data (pos + normal)
    std::vector<float> vdata;
    vdata.reserve(vertices.size() * 6);
    for (size_t i = 0; i < vertices.size(); ++i) {
        vdata.push_back(vertices[i].currPos.x);
        vdata.push_back(vertices[i].currPos.y);
        vdata.push_back(vertices[i].currPos.z);
        vdata.push_back(vertices[i].currNormal.x);
        vdata.push_back(vertices[i].currNormal.y);
        vdata.push_back(vertices[i].currNormal.z);
    }

    // Build indices vector explicitly (safe)
    std::vector<unsigned int> indices;
    indices.reserve(triangles.size() * 3);
    for (size_t t = 0; t < triangles.size(); ++t) {
        indices.push_back(triangles[t].v0);
        indices.push_back(triangles[t].v1);
        indices.push_back(triangles[t].v2);
    }


    // Create temporary VAO/VBO/EBO (same style as Joint::Draw)
    GLuint tmpVAO = 0, tmpVBO = 0, tmpEBO = 0;
    glGenVertexArrays(1, &tmpVAO);
    glGenBuffers(1, &tmpVBO);
    glGenBuffers(1, &tmpEBO);

    glBindVertexArray(tmpVAO);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, tmpVBO);
    glBufferData(GL_ARRAY_BUFFER, vdata.size() * sizeof(float), vdata.data(), GL_STATIC_DRAW);

    // EBO
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, tmpEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Attributes: location 0 = position, location 1 = normal
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));

    // Draw
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(indices.size()), GL_UNSIGNED_INT, 0);

    // Cleanup
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glDeleteBuffers(1, &tmpVBO);
    glDeleteBuffers(1, &tmpEBO);
    glDeleteVertexArrays(1, &tmpVAO);

    glUseProgram(0);
}






