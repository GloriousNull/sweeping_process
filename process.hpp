//
// Created by GloriousSir on 1/20/2022.
//

#ifndef SWEEPING_PROCESS_PROCESS_HPP
#define SWEEPING_PROCESS_PROCESS_HPP

#include <future>
#include "math/float2.hpp"

struct translation_physics_movement_data
{
    math::float2 position;
    math::float2 velocity;
    float radius;
    float mass;
};

struct projection_and_distance_multiplier
{
    math::float2 projection_position;
    float multiplayer;
};

float distance(math::float2, math::float2);

void initialize_outer_rings(math::float2, translation_physics_movement_data *);
void initialize_inner_rings(math::float2, translation_physics_movement_data *, float);
void update_rings_position(translation_physics_movement_data *, float, std::future<void> *);
void update_rings(translation_physics_movement_data *out, translation_physics_movement_data *outer_cups_buffer, std::future<void> *handles,
                  const float delta_time);

projection_and_distance_multiplier calculate_projection(math::float2, math::float2, float);

#endif //SWEEPING_PROCESS_PROCESS_HPP
