#include "Skeleton.h"

#include "Joint.h"

Skeleton::Skeleton() {
	root = nullptr;
}

Skeleton::~Skeleton() {
	delete root;
	root = nullptr;
    joints.clear();
}

bool Skeleton::Load(const char* file) {
    Tokenizer token;

    token.Open(file);


    // Find first "balljoint"
    token.FindToken("balljoint");


    // Read root joint name
    char rootName[256];
    token.GetToken(rootName);
    

    // Create root joint and load it
    root = new Joint();
    root->SetName(rootName);
    root->Load(token);

    token.Close();

    //project 2
    joints.clear();
    BuildJointList(root);

    Update();
    return true;
}

void Skeleton::Update() {
	root->Update(glm::mat4(1.0f));
}

void Skeleton:: Draw(const glm::mat4& viewProjMtx, GLuint shader) {
	root->Draw(viewProjMtx, shader);
}

void Skeleton::BuildJointList(Joint* joint) {
    joint->SetIndex((int)joints.size());
    joints.push_back(joint);

    for (Joint* child : joint->GetChildren()) {
        BuildJointList(child);
    }
}

glm::mat4 Skeleton::GetWorldMatrix(int joint) const {
    if (joint < 0 || joint >= (int)joints.size()) {
        return glm::mat4(1.0f);
    }
    return joints[joint]->GetWorldMatrix();
}

void Skeleton::DrawDOFGui() {
    if (!root) return;
    root->DrawDOFGui();
}

Joint* Skeleton::getRoot() {
    return root;
}


