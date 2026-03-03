#include "AnimationClip.h"
#include "Skeleton.h"
#include "Joint.h"
#include <iostream>
#include <cstring>

AnimationClip::AnimationClip() {
    timeStart = 0.0f;
    timeEnd = 0.0f;
}

bool AnimationClip::Load(const char* filename) {
    Tokenizer token;

    if (!token.Open(filename)) {
        std::cerr << "Failed to open animation file: " << filename << std::endl;
        return false;
    }

    std::cout << "Loading animation: " << filename << std::endl;

    // Find "animation" token
    if (!token.FindToken("animation")) {
        std::cerr << "Could not find 'animation' token" << std::endl;
        token.Close();
        return false;
    }

    if (!token.FindToken("{")) {
        std::cerr << "Could not find opening brace for animation" << std::endl;
        token.Close();
        return false;
    }

    // Parse animation contents
    int currentChannelIndex = 0;  // Track which channel we're loading

    while (true) {
        char temp[256];
        token.GetToken(temp);

        if (strcmp(temp, "range") == 0) {
            timeStart = token.GetFloat();
            timeEnd = token.GetFloat();
            std::cout << "Animation range: " << timeStart << " to " << timeEnd << std::endl;
        }
        else if (strcmp(temp, "numchannels") == 0) {
            int numChannels = token.GetInt();
            channels.resize(numChannels);
            std::cout << "Number of channels: " << numChannels << std::endl;
        }
        else if (strcmp(temp, "channel") == 0) {
            // Read and verify channel number
            int fileChannelNum = token.GetInt();

            if (fileChannelNum != currentChannelIndex) {
                std::cerr << "Warning: Expected channel " << currentChannelIndex
                    << " but found channel " << fileChannelNum << std::endl;
            }


            if (!channels[currentChannelIndex].Load(token)) {
                std::cerr << "Failed to load channel " << currentChannelIndex << std::endl;
                token.Close();
                return false;
            }

            currentChannelIndex++;
        }
        else if (strcmp(temp, "}") == 0) {
            // End of animation block
            std::cout << "Successfully loaded animation with " << channels.size() << " channels" << std::endl;
            token.Close();
            return true;
        }
        else {
            token.SkipLine();
        }
    }

    token.Close();
    return false;
}

void AnimationClip::Evaluate(float time, Skeleton* skeleton) {
    if (!skeleton || channels.empty()) {
        return;
    }

    // The first 3 channels are root translation (x, y, z)
    // The remaining channels are joint rotations in depth-first order
    // Each joint has 3 channels (x, y, z rotations)

    int channelIndex = 0;
    Joint* root = skeleton->getRoot();

    // Apply root translation (channels 0, 1, 2)
    if (root && channelIndex + 2 < (int)channels.size()) {
        float tx = channels[0].Evaluate(time);
        float ty = channels[1].Evaluate(time);
        float tz = channels[2].Evaluate(time);

        // Apply animation offset to root joint
        root->SetAnimationOffset(glm::vec3(tx, ty, tz));

        channelIndex = 3;
    }

    // Evaluate joint rotations starting from root
    EvaluateJointChannels(root, channelIndex, time);
}

void AnimationClip::EvaluateJointChannels(Joint* joint, int& channelIndex, float time) {
    if (!joint) return;

    // Each joint has 3 DOF channels (x, y, z rotations)
    if (channelIndex + 2 < (int)channels.size()) {
        float x = channels[channelIndex].Evaluate(time);
        float y = channels[channelIndex + 1].Evaluate(time);
        float z = channels[channelIndex + 2].Evaluate(time);

        joint->SetDOF(0, x);  // X rotation
        joint->SetDOF(1, y);  // Y rotation
        joint->SetDOF(2, z);  // Z rotation

        channelIndex += 3;
    }

    // Recursively process children in order
    const std::vector<Joint*>& children = joint->GetChildren();
    for (Joint* child : children) {
        EvaluateJointChannels(child, channelIndex, time);
    }
}