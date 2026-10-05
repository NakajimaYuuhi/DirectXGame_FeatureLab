#pragma once
#include <cmath>
#include <numbers>

enum class Ease
{
    Linear,
    InSine,
    OutSine,
    InOutSine,
    InQuad,
    OutQuad,
    InOutQuad,
    InCubic,
    OutCubic,
    InOutCubic,
    InQuart,
    OutQuart,
    InOutQuart,
    InQuint,
    OutQuint,
    InOutQuint,
    InExpo,
    OutExpo,
    InOutExpo,
    InBack,
    OutBack,
    InOutBack,
    InBounce,
    OutBounce,
    InOutBounce,
    InElastic,
    OutElastic,
    InOutElastic
};

class EaseUtility
{
public:
    static float Evaluate(Ease ease, float t)
    {
        // Clamp t to [0, 1]
        if (t <= 0.0f) return 0.0f;
        if (t >= 1.0f) return 1.0f;

        constexpr float PI = 3.14159265358979323846f;
        constexpr float c1 = 1.70158f;
        constexpr float c2 = c1 * 1.525f;
        constexpr float c3 = c1 + 1.0f;
        constexpr float c4 = (2.0f * PI) / 3.0f;
        constexpr float c5 = (2.0f * PI) / 4.5f;

        switch (ease)
        {
        case Ease::Linear:
            return t;

        // Sine
        case Ease::InSine:
            return 1.0f - std::cos((t * PI) * 0.5f);
        case Ease::OutSine:
            return std::sin((t * PI) * 0.5f);
        case Ease::InOutSine:
            return -(std::cos(PI * t) - 1.0f) * 0.5f;

        // Quad
        case Ease::InQuad:
            return t * t;
        case Ease::OutQuad:
            return 1.0f - (1.0f - t) * (1.0f - t);
        case Ease::InOutQuad:
            return (t < 0.5f) ? (2.0f * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) * 0.5f);

        // Cubic
        case Ease::InCubic:
            return t * t * t;
        case Ease::OutCubic:
            return 1.0f - std::pow(1.0f - t, 3.0f);
        case Ease::InOutCubic:
            return (t < 0.5f) ? (4.0f * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) * 0.5f);

        // Quart
        case Ease::InQuart:
            return t * t * t * t;
        case Ease::OutQuart:
            return 1.0f - std::pow(1.0f - t, 4.0f);
        case Ease::InOutQuart:
            return (t < 0.5f) ? (8.0f * t * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) * 0.5f);

        // Quint
        case Ease::InQuint:
            return t * t * t * t * t;
        case Ease::OutQuint:
            return 1.0f - std::pow(1.0f - t, 5.0f);
        case Ease::InOutQuint:
            return (t < 0.5f) ? (16.0f * t * t * t * t * t) : (1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) * 0.5f);

        // Expo
        case Ease::InExpo:
            return std::pow(2.0f, 10.0f * t - 10.0f);
        case Ease::OutExpo:
            return 1.0f - std::pow(2.0f, -10.0f * t);
        case Ease::InOutExpo:
            return (t < 0.5f)
                ? (std::pow(2.0f, 20.0f * t - 10.0f) * 0.5f)
                : ((2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) * 0.5f);

        // Back
        case Ease::InBack:
            return c3 * t * t * t - c1 * t * t;
        case Ease::OutBack:
            return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
        case Ease::InOutBack:
            return (t < 0.5f)
                ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) * 0.5f
                : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) * 0.5f;

        // Bounce
        case Ease::OutBounce:
            return OutBounceInternal(t);
        case Ease::InBounce:
            return 1.0f - OutBounceInternal(1.0f - t);
        case Ease::InOutBounce:
            return (t < 0.5f)
                ? (1.0f - OutBounceInternal(1.0f - 2.0f * t)) * 0.5f
                : (1.0f + OutBounceInternal(2.0f * t - 1.0f)) * 0.5f;

        // Elastic
        case Ease::InElastic:
            return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
        case Ease::OutElastic:
            return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
        case Ease::InOutElastic:
            return (t < 0.5f)
                ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f
                : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) * 0.5f + 1.0f;

        default:
            return t;
        }
    }

private:
    static float OutBounceInternal(float x)
    {
        constexpr float n1 = 7.5625f;
        constexpr float d1 = 2.75f;

        if (x < 1.0f / d1)
        {
            return n1 * x * x;
        }
        else if (x < 2.0f / d1)
        {
            x -= 1.5f / d1;
            return n1 * x * x + 0.75f;
        }
        else if (x < 2.5f / d1)
        {
            x -= 2.25f / d1;
            return n1 * x * x + 0.9375f;
        }
        else
        {
            x -= 2.625f / d1;
            return n1 * x * x + 0.984375f;
        }
    }
};