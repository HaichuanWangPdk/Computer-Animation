#include <vector>

#include "Tokenizer.h"

#include "Channel.h"

Channel::Channel() {
    before = "constant";
    after = "constant";
}

bool Channel::Load(Tokenizer& t) {
    if (!t.FindToken("{")) {
        return false;
    }

    // Now parse everything inside { }
    while (true) {
        char temp[256];
        t.GetToken(temp);

        if (strcmp(temp, "extrapolate") == 0) {
            // Read two extrapolation mode strings
            char extrap_in[256];
            char extrap_out[256];
            t.GetToken(extrap_in);
            t.GetToken(extrap_out);
            before = extrap_in;
            after = extrap_out;
        } else if (strcmp(temp, "keys") == 0) {
            keys.resize(t.GetInt());

            if (!t.FindToken("{")) {
                return false;
            }

            for (int i = 0; i < keys.size(); i++) {
                keys[i].Time = t.GetFloat();
                keys[i].Value = t.GetFloat();

                char rule_in_temp[256];
                t.GetToken(rule_in_temp);
                if ((strcmp(rule_in_temp, "smooth") == 0) || (strcmp(rule_in_temp, "flat") == 0) || (strcmp(rule_in_temp, "linear") == 0)) {
                    keys[i].RuleIn = rule_in_temp;
                    keys[i].TangentIn = 0.0f;
                } else {
                    keys[i].RuleIn = "fixed";
                    keys[i].TangentIn = (float)atof(rule_in_temp);
                }

                char rule_out_temp[256];
                t.GetToken(rule_out_temp);
                if ((strcmp(rule_out_temp, "smooth") == 0) || (strcmp(rule_out_temp, "flat") == 0) || (strcmp(rule_out_temp, "linear") == 0)) {
                    keys[i].RuleOut = rule_out_temp;
                    keys[i].TangentOut = 0.0f;
                }
                else {
                    keys[i].RuleOut = "fixed";
                    keys[i].TangentOut = (float)atof(rule_out_temp);
                }
            }

            if (!t.FindToken("}")) {
                return false;
            }

        }
        else if (strcmp(temp, "}") == 0) {
            // Reached closing brace for channel
            // Now precompute all tangents and cubic coefficients
            Precompute();
            return true;
        }
        else {
            // Unknown token - skip this line
            t.SkipLine();
        }
    }

    return false;
}

void Channel::Precompute() {
    // First pass: compute tangents based on rules
    for (int i = 0; i < (int)keys.size(); i++) {
        ComputeTangent(i);
    }

    // Second pass: compute cubic coefficients for each span
    for (int i = 0; i < (int)keys.size() - 1; i++) {
        ComputeCubicCoefficients(i);
    }
}

void Channel::ComputeTangent(int keyIndex) {
    Keyframe& key = keys[keyIndex];
    int n = (int)keys.size();

    // Compute incoming tangent
    if (key.RuleIn == "flat") {
        key.TangentIn = 0.0f;
    }
    else if (key.RuleIn == "linear") {
        if (keyIndex > 0) {
            float dt = key.Time - keys[keyIndex - 1].Time;
            float dv = key.Value - keys[keyIndex - 1].Value;
            key.TangentIn = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else {
            key.TangentIn = 0.0f;
        }
    }
    else if (key.RuleIn == "smooth") {
        if (keyIndex > 0 && keyIndex < n - 1) {
            float dt = keys[keyIndex + 1].Time - keys[keyIndex - 1].Time;
            float dv = keys[keyIndex + 1].Value - keys[keyIndex - 1].Value;
            key.TangentIn = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else if (keyIndex == 0 && n > 1) {
            // First key - use linear rule
            float dt = keys[1].Time - keys[0].Time;
            float dv = keys[1].Value - keys[0].Value;
            key.TangentIn = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else if (keyIndex == n-1 && n > 1) {
            // Last key - use linear rule
            float dt = keys[n-1].Time - keys[n-2].Time;
            float dv = keys[n-1].Value - keys[n-2].Value;
            key.TangentIn = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else {
            key.TangentIn = 0.0f;
        }
    }
    // else it's "fixed" and TangentIn was already set during Load

    // Compute outgoing tangent
    if (key.RuleOut == "flat") {
        key.TangentOut = 0.0f;
    }
    else if (key.RuleOut == "linear") {
        if (keyIndex < n - 1) {
            float dt = keys[keyIndex + 1].Time - key.Time;
            float dv = keys[keyIndex + 1].Value - key.Value;
            key.TangentOut = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else {
            key.TangentOut = 0.0f;
        }
    }
    else if (key.RuleOut == "smooth") {
        if (keyIndex > 0 && keyIndex < n - 1) {
            float dt = keys[keyIndex + 1].Time - keys[keyIndex - 1].Time;
            float dv = keys[keyIndex + 1].Value - keys[keyIndex - 1].Value;
            key.TangentOut = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else if (keyIndex == 0 && n > 1) {
            // First key - use linear rule
            float dt = keys[1].Time - keys[0].Time;
            float dv = keys[1].Value - keys[0].Value;
            key.TangentOut = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else if (keyIndex == n - 1 && n > 1) {
            // Last key - use linear rule
            float dt = keys[n - 1].Time - keys[n - 2].Time;
            float dv = keys[n - 1].Value - keys[n - 2].Value;
            key.TangentOut = (dt != 0.0f) ? (dv / dt) : 0.0f;
        }
        else {
            key.TangentOut = 0.0f;
        }
    }
    // else it's "fixed" and TangentOut was already set during Load
}

void Channel::ComputeCubicCoefficients(int keyIndex) {
    // Compute cubic coefficients for the span from keyIndex to keyIndex+1
    const Keyframe& k0 = keys[keyIndex];
    const Keyframe& k1 = keys[keyIndex + 1];

    float p0 = k0.Value;
    float p1 = k1.Value;
    float dt = k1.Time - k0.Time;

    // Scale tangents by time interval to normalize to 0..1 range
    float v0 = k0.TangentOut * dt;
    float v1 = k1.TangentIn * dt;

    // Hermite basis matrix multiplication
    // [A] =  [ 2 -2  1  1] [p0]
    // [B] =  [-3  3 -2 -1] [p1]
    // [C] =  [ 0  0  1  0] [v0]
    // [D] =  [ 1  0  0  0] [v1]

    keys[keyIndex].A = 2.0f * p0 - 2.0f * p1 + v0 + v1;
    keys[keyIndex].B = -3.0f * p0 + 3.0f * p1 - 2.0f * v0 - v1;
    keys[keyIndex].C = v0;
    keys[keyIndex].D = p0;
}

int Channel::FindSpan(float time) const {
    // Binary search for the span containing time
    int n = (int)keys.size();

    if (n == 0) return -1;
    if (time < keys[0].Time) return -1;
    if (time >= keys[n - 1].Time) return n;

    // Binary search
    int low = 0;
    int high = n - 1;

    while (low < high - 1) {
        int mid = (low + high) / 2;
        if (time < keys[mid].Time) {
            high = mid;
        }
        else {
            low = mid;
        }
    }

    return low;
}

float Channel::EvaluateSpan(int span, float time) const {
    const Keyframe& k = keys[span];
    const Keyframe& k_next = keys[span + 1];

    // Normalize time to 0..1 range for the span
    float dt = k_next.Time - k.Time;
    float u = (dt != 0.0f) ? (time - k.Time) / dt : 0.0f;

    // Evaluate cubic: f(u) = Au^3 + Bu^2 + Cu + D
    // Horner's method: f(u) = D + u(C + u(B + u*A))
    float result = k.D + u * (k.C + u * (k.B + u * k.A));

    return result;
}

float Channel::Extrapolate(float time, bool isBefore) const {
    int n = (int)keys.size();
    if (n == 0) return 0.0f;
    if (n == 1) return keys[0].Value;

    // FIX: Use 'before' and 'after' not 'extrapIn' and 'extrapOut'
    const std::string& mode = isBefore ? before : after;

    if (mode == "constant") {
        return isBefore ? keys[0].Value : keys[n - 1].Value;
    }
    else if (mode == "linear") {
        if (isBefore) {
            float slope = keys[0].TangentOut;
            return keys[0].Value + slope * (time - keys[0].Time);
        }
        else {
            float slope = keys[n - 1].TangentIn;
            return keys[n - 1].Value + slope * (time - keys[n - 1].Time);
        }
    }
    else if (mode == "cycle") {
        float t0 = keys[0].Time;
        float t1 = keys[n - 1].Time;
        float duration = t1 - t0;

        if (duration == 0.0f) return keys[0].Value;

        // Wrap time into the range [t0, t1)
        float offset = time - t0;
        float cycles = floor(offset / duration);
        float wrapped = t0 + (offset - cycles * duration);

        // Recursively evaluate at wrapped time
        int span = FindSpan(wrapped);
        if (span >= 0 && span < n - 1) {
            return EvaluateSpan(span, wrapped);
        }
        return keys[0].Value;
    }
    else if (mode == "cycle_offset") {
        float t0 = keys[0].Time;
        float t1 = keys[n - 1].Time;
        float duration = t1 - t0;

        if (duration == 0.0f) return keys[0].Value;

        float offset = time - t0;
        float cycles = floor(offset / duration);
        float wrapped = t0 + (offset - cycles * duration);

        // Evaluate at wrapped time
        float base_value;
        int span = FindSpan(wrapped);
        if (span >= 0 && span < n - 1) {
            base_value = EvaluateSpan(span, wrapped);
        }
        else {
            base_value = keys[0].Value;
        }

        // Add offset based on number of cycles
        float value_offset = (keys[n - 1].Value - keys[0].Value) * cycles;
        return base_value + value_offset;
    }
    else if (mode == "bounce") {
        float t0 = keys[0].Time;
        float t1 = keys[n - 1].Time;
        float duration = t1 - t0;

        if (duration == 0.0f) return keys[0].Value;

        float offset = time - t0;
        float cycles = floor(offset / duration);
        int cycle_num = (int)cycles;
        float t_in_cycle = offset - cycles * duration;

        float wrapped;
        if (cycle_num % 2 == 0) {
            // Forward
            wrapped = t0 + t_in_cycle;
        }
        else {
            // Backward
            wrapped = t1 - t_in_cycle;
        }

        // Evaluate at wrapped time
        int span = FindSpan(wrapped);
        if (span >= 0 && span < n - 1) {
            return EvaluateSpan(span, wrapped);
        }
        else if (span == -1) {
            return keys[0].Value;
        }
        else {
            return keys[n - 1].Value;
        }
    }

    // Default to constant
    return isBefore ? keys[0].Value : keys[n - 1].Value;
}

float Channel::Evaluate(float time) const {
    int n = (int)keys.size();

    // Handle empty or single key cases
    if (n == 0) return 0.0f;
    if (n == 1) return keys[0].Value;

    // Find the span containing time
    int span = FindSpan(time);

    if (span == -1) {
        // Before first key - use extrapolation
        return Extrapolate(time, true);
    }
    else if (span >= n - 1) {
        // After last key - use extrapolation
        return Extrapolate(time, false);
    }
    else {
        // Within valid range - evaluate cubic
        return EvaluateSpan(span, time);
    }
}