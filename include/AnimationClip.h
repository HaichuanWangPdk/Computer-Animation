#pragma once

#include <vector>

#include "Channel.h"

#include "Tokenizer.h"

// Forward declaration
class Skeleton;
class Joint;

class AnimationClip {
private:
    std::vector<Channel> channels;
    float timeStart;
    float timeEnd;

    void EvaluateJointChannels(Joint* joint, int& channelIndex, float time);

public:
    AnimationClip();

    bool Load(const char* filename);
    void Evaluate(float time, Skeleton* skeleton);

    float GetStartTime() const { return timeStart; }
    float GetEndTime() const { return timeEnd; }
    float GetDuration() const { return timeEnd - timeStart; }
};