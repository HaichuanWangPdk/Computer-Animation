#pragma once

#include <vector>
#include <string>
#include "Tokenizer.h"

struct Keyframe {
    float Time;
    float Value;
    float TangentIn, TangentOut;
    std::string RuleIn, RuleOut;  // Tangent rules: "flat", "linear", "smooth", or "fixed"
    float A, B, C, D;              // Cubic coefficients
};

class Channel {
private:
    std::vector<Keyframe> keys;
    std::string before;   // Extrapolation mode before first key
    std::string after;  // Extrapolation mode after last key

    // Helper functions
    void Precompute();
    void ComputeTangent(int keyIndex);
    void ComputeCubicCoefficients(int keyIndex);
    int FindSpan(float time) const;
    float EvaluateSpan(int span, float time) const;
    float Extrapolate(float time, bool before) const;

public:
    Channel();

    float Evaluate(float time) const;
    bool Load(Tokenizer& t);
};