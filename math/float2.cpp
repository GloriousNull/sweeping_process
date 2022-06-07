//
// Created by GloriousSir on 1/21/2022.
//

#include <cmath>
#include "float2.hpp"

namespace math
{
    float2 add(const float2 first, const float2 second)
    {
        return {first.x + second.x, first.y + second.y};
    }

    float2 operator+(const float2 first, const float2 second)
    {
        return {first.x + second.x, first.y + second.y};
    }

    float2 subtract(const float2 first, const float2 second)
    {
        return {first.x - second.x, first.y - second.y};
    }

    float2 operator-(const float2 first, const float2 second)
    {
        return {first.x - second.x, first.y - second.y};
    }

    float dot_product(const float2 first, const float2 second)
    {
        return first.x * second.x + first.y * second.y;
    }

    float2 multiply(const float2 point, const float number)
    {
        return {point.x * number, point.y * number};
    }

    float2 operator*(const float2 point, const float number)
    {
        return {point.x * number, point.y * number};
    }

    float2 divide(const float2 point, const float number)
    {
        return {point.x / number, point.y / number};
    }

    float2 operator/(const float2 point, const float number)
    {
        return {point.x / number, point.y / number};
    }

    float2 normalise(const float2 vec)
    {
        return divide(vec, length(vec));
    }

    float length(const float2 vec)
    {
        return sqrtf(dot_product(vec, vec));
    }

    float distance(const float2 first, const float2 second)
    {
        const auto x = first.x - second.x;
        const auto y = first.y - second.y;

        return sqrtf(x * x + y * y);
    }

    float distance_square(const float2 first, const float2 second)
    {
        const auto x = first.x - second.x;
        const auto y = first.y - second.y;

        return x * x + y * y;
    }
}