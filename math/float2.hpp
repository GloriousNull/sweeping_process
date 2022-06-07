//
// Created by GloriousSir on 1/21/2022.
//

#ifndef SWEEPING_PROCESS_FLOAT2_HPP
#define SWEEPING_PROCESS_FLOAT2_HPP

namespace math
{
    struct uint2
    {
        uint32_t x, y;
    };

    struct float2
    {
        float x, y;
    };

    float2 add(float2, float2);
    float2 operator+(float2, float2);
    float2 subtract(float2, float2);
    float2 operator-(float2, float2);
    float dot_product(float2, float2);
    float2 multiply(float2, float);
    float2 operator*(float2, float);
    float2 divide(float2, float);
    float2 operator/(float2, float);
    float2 normalise(float2);
    float length(float2);
    float distance(float2, float2);
    float distance_square(float2, float2);
}

#endif //SWEEPING_PROCESS_FLOAT2_HPP
