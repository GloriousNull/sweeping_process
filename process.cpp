//
// Created by GloriousSir on 1/20/2022.
//

#include <cmath>
#include <numbers>
#include "process.hpp"
#include "ui.hpp"

using namespace math;

#include <random>
#include <iostream>

#include "threads/BS_thread_pool.hpp"

#include "constants.hpp"

extern BS::thread_pool scheduler;

std::random_device seed;
std::mt19937 generator{seed()};

std::uniform_int_distribution<uint32_t> x_distribution(0, screen_size.x);
std::uniform_int_distribution<uint32_t> y_distribution(0, screen_size.y);

std::uniform_int_distribution<int32_t> radius_offset(-2, 2);
std::uniform_int_distribution<int32_t> speed_offset(1, 5);

#include "profile/optick.h"

projection_and_distance_multiplier calculate_projection(const float2 point, const float2 ring_center, const float radius)
{
    OPTICK_EVENT();
    // ax+by+c=0, c always 0
    const float a = point.y - ring_center.y;
    const float b = point.x - ring_center.x;

    const float ring_radius_to_point_radius_ratio = sqrtf(radius * radius / (a * a + b * b));

    float2 projection{b * ring_radius_to_point_radius_ratio + ring_center.x, a * ring_radius_to_point_radius_ratio + ring_center.y};

    return {projection, ring_radius_to_point_radius_ratio};
}

void initialize_outer_rings(const float2 origin, translation_physics_movement_data * out)
{
    const float2 center = origin + float2{1, 1} * outer_cup_radius * sqrtf(outer_cups_size);
    float2 next = origin;
    std::size_t row{1};
    std::size_t row_size = sqrtf(outer_cups_size);
    for (std::size_t i{0}; i < outer_cups_size; ++i)
    {
        const translation_physics_movement_data cup
        {
        .position = next,
        .velocity = math::normalise(center - next) * static_cast<float>(speed_offset(generator)) * 250,
        .radius = outer_cup_radius,
        .mass = 200
        };

        out[i] = cup;

        if ((i + 1) % row_size != 0)
            next = next + float2{1, 0} * outer_cup_radius * 2.5f;
        else
            next = origin + float2{0, 1} * outer_cup_radius * 2.5f * static_cast<float>(row++);
    }
}

void initialize_inner_rings(const float2 origin, translation_physics_movement_data * out, const float ring_radius)
{
    float angle_step = std::numbers::pi / 6;

    float distance_from_origin = ring_radius * 6;

    for (std::size_t i{0}; i < 3; ++i)
    {
        const float delta_size = static_cast<float>(radius_offset(generator));
        float angle{0.0f};

        for (size_t j{0}; j < 12; ++j)
        {
            const size_t current = j + i * 12;

            const translation_physics_movement_data cup
            {
            .position = origin + float2{cos(angle) * distance_from_origin, sin(angle)
                                                                           * distance_from_origin},
            .velocity =math::normalise({static_cast<float>(x_distribution(generator)),
                                        static_cast<float>(y_distribution(generator))}) * 120,
            .radius = ring_radius + delta_size,
            .mass = 5 + delta_size
            };

            out[current] = cup;

            angle += angle_step;
        }

        distance_from_origin += ring_radius * 2.5f;
    }

    angle_step = 2.0f * std::numbers::pi_v<float> / static_cast<float>(inner_cups_row_size);

    for (std::size_t i{0}; i < inner_cups_rows_size; ++i)
    {
        const float delta_size = static_cast<float>(radius_offset(generator));
        float angle{0.0f};

        for (size_t j{0}; j < inner_cups_row_size; ++j)
        {
            const size_t current = j + 36 + i * inner_cups_row_size;

            const translation_physics_movement_data cup
            {
            .position = origin + float2{cos(angle) * distance_from_origin, sin(angle)
                                                                           * distance_from_origin},
            .velocity =math::normalise({static_cast<float>(x_distribution(generator)),
                                        static_cast<float>(y_distribution(generator))}) * 120,
            .radius = ring_radius + delta_size,
            .mass = 5 + delta_size
            };

            out[current] = cup;

            angle += angle_step;
        }

        distance_from_origin += ring_radius * 2.5f;
    }
}

void update_rings_position(translation_physics_movement_data * out, const float delta_time, std::future<void> * handles)
{
    OPTICK_EVENT();

    for (std::size_t i{0}; i < jobs_size; ++i)
    {
        handles[i] = scheduler.submit
        ([
        out = out + i * jobs_rings_size_step,
        delta_time
        ]()
        {
            for (std::size_t i{0}; i < jobs_rings_size_step; ++i)
                out[i].position = out[i].position + out[i].velocity * delta_time;
        });
    }

    for (std::size_t i{jobs_size * jobs_rings_size_step}; i < cups_size; ++i)
        out[i].position = out[i].position + out[i].velocity * delta_time;

    for (std::size_t i{0}; i < jobs_size; ++i)
        handles[i].wait();
}

void update_inner_rings_physics(translation_physics_movement_data * out, const std::size_t begin_it, const std::size_t end_it)
{
    OPTICK_EVENT();
    for (std::size_t i{begin_it}; i < end_it; ++i)
    {
        const translation_physics_movement_data outer = out[i];

        for (std::size_t j{0}; j < inner_cups_padding; ++j)
        {
            const std::size_t first_inner_index{j + outer_cups_size + i * inner_cups_padding};
            translation_physics_movement_data first = out[first_inner_index];

            for (std::size_t k{0}; k < inner_cups_padding; ++k)
            {
                if (j == k)
                    continue;

                const std::size_t second_inner_index{k + outer_cups_size + i * inner_cups_padding};
                translation_physics_movement_data second = out[second_inner_index];

                const float delta_x = first.position.x - second.position.x;
                const float delta_y = first.position.y - second.position.y;

                const float distance = std::sqrtf(delta_x * delta_x + delta_y * delta_y);

                const float radius_sum = first.radius + second.radius;

                if (distance < radius_sum)
                {
                    // статическая коллизия
                    const float overlap_distance = 0.5f * (distance - radius_sum);

                    const float x_normal = delta_x / distance;
                    const float y_normal = delta_y / distance;

                    const float overlap_delta_x = x_normal * overlap_distance;
                    const float overlap_delta_y = y_normal * overlap_distance;

                    const float2 position_delta{overlap_delta_x, overlap_delta_y};

                    first.position = first.position - position_delta;
                    second.position = second.position + position_delta;

                    // динамическая коллизия
                    const float2 delta_velocity = first.velocity - second.velocity;
                    const float p = x_normal * delta_velocity.x + y_normal * delta_velocity.y;
                    first.velocity.x -= p * x_normal;
                    first.velocity.y -= p * y_normal;
                    second.velocity.x += p * x_normal;
                    second.velocity.y += p * y_normal;

                    out[second_inner_index] = second;
                }
            }

            const float2 collision_point = first.position + math::normalise(first.position - outer.position) * first.radius;
            const projection_and_distance_multiplier projection_result = calculate_projection(collision_point,
                                                                                              outer.position, outer.radius);
            if (projection_result.multiplayer <= 1)
            {
                first.position = first.position + projection_result.projection_position - collision_point;

                const float2 delta_velocity = first.velocity - outer.velocity;
                const float2 normal_vector = math::normalise(delta_velocity);
                const float p = 2.0f * (normal_vector.x * delta_velocity.x + normal_vector.y * delta_velocity.y) /
                                (first.mass + outer.mass);
                first.velocity = first.velocity - normal_vector * p * outer.mass;
            }

            out[first_inner_index] = first;
        }
    }
}

void update_rings_physics(translation_physics_movement_data * out, translation_physics_movement_data * outer_cups_buffer, std::future<void> * handles)
{
    OPTICK_EVENT();

    for (std::size_t i{0}; i < outer_cups_size; ++i)
        outer_cups_buffer[i] = out[i];

    std::size_t begin_it;
    std::size_t end_it{0};

    for (std::size_t i{0}; i < jobs_size; ++i)
    {
        begin_it = end_it;
        end_it += jobs_cups_size_step;
        handles[i] = scheduler.submit
        ([
        out,
        begin_it,
        end_it
        ]()
        {
            update_inner_rings_physics(out, begin_it, end_it);
        });
    }

    begin_it = end_it;
    end_it = outer_cups_size;

//    auto h = scheduler.submit
//    ([
//     out,
//     begin_it,
//     end_it
//     ]()
//     {
//         update_inner_rings_physics(out, begin_it, end_it);
//     });

    update_inner_rings_physics(out, begin_it, end_it);

    for (std::size_t i{0}; i < outer_cups_size; ++i)
    {
        translation_physics_movement_data first = outer_cups_buffer[i];

        for (std::size_t j{0}; j < outer_cups_size; ++j)
        {
            if (i == j)
                continue;

            translation_physics_movement_data second = outer_cups_buffer[j];

            const float delta_x = first.position.x - second.position.x;
            const float delta_y = first.position.y - second.position.y;

            const float distance = std::sqrtf(delta_x * delta_x + delta_y * delta_y);

            const float radius_sum = first.radius + second.radius;

            if (distance < radius_sum)
            {
                // статическая коллизия
                const float overlap_distance = 0.5f * (distance - radius_sum);

                const float x_normal = delta_x / distance;
                const float y_normal = delta_y / distance;

                const float overlap_delta_x = x_normal * overlap_distance;
                const float overlap_delta_y = y_normal * overlap_distance;

                const float2 position_delta{overlap_delta_x, overlap_delta_y};

                first.position = first.position - position_delta;
                second.position = second.position + position_delta;

                // динамическая коллизия
                const float2 delta_velocity = first.velocity - second.velocity;
                const float p = x_normal * delta_velocity.x + y_normal * delta_velocity.y;
                first.velocity.x -= p * x_normal;
                first.velocity.y -= p * y_normal;
                second.velocity.x += p * x_normal;
                second.velocity.y += p * y_normal;

                outer_cups_buffer[j] = second;
            }

            outer_cups_buffer[i] = first;
        }

    }

    OPTICK_EVENT("waiting_update");
    for (std::size_t i{0}; i < jobs_size; ++i)
        handles[i].wait();

//    h.wait();

    for (std::size_t i{0}; i < outer_cups_size; ++i)
        out[i] = outer_cups_buffer[i];
}
