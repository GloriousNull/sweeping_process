//
// Created by GloriousSir on 1/20/2022.
//

#include <cmath>
#include <numbers>
#include <random>

#include "process.hpp"
#include "ui.hpp"

#include "threads/BS_thread_pool.hpp"

#include "constants.hpp"

using namespace math;

extern BS::thread_pool scheduler;

std::random_device seed;
std::mt19937 generator{seed()};

std::uniform_int_distribution<uint32_t> x_distribution(0, screen_size.x);
std::uniform_int_distribution<uint32_t> y_distribution(0, screen_size.y);

std::uniform_int_distribution<int32_t> radius_offset(-2, 2);
std::uniform_int_distribution<int32_t> speed_offset(1, 5);

#include "profile/optick.h"

projection_and_distance_multiplier calculate_projection(const float2 point,const float2 ring_center,
                                                        const float ring_radius)
{
    OPTICK_EVENT();
    // ax+by+c=0, c always 0
    const float a = point.y - ring_center.y;
    const float b = point.x - ring_center.x;

    const float ring_radius_to_point_radius_ratio = sqrtf(ring_radius * ring_radius / (a * a + b * b));

    const float2 projection{b * ring_radius_to_point_radius_ratio + ring_center.x,
                      a * ring_radius_to_point_radius_ratio + ring_center.y};

    return {projection, ring_radius_to_point_radius_ratio};
}

void initialize_outer_rings(const float2 origin, translation_physics_movement_data * out)
{
    const float2 center = origin + float2{1, 1} * outer_cup_radius * sqrtf(outer_cups_size) * 1.1f;
    float2 next = origin;
    std::size_t row{1};
    std::size_t row_size = sqrtf(outer_cups_size);
    for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
    {
        const translation_physics_movement_data cup
        {
        .position = next,
        .velocity = math::normalise(center - next) * static_cast<float>(speed_offset(generator)) * 250,
        .radius = outer_cup_radius,
        .mass = 200
        };

        out[outer_it] = cup;

        if ((outer_it + 1) % row_size != 0)
            next = next + float2{1, 0} * outer_cup_radius * 2.5f;
        else
            next = origin + float2{0, 1} * outer_cup_radius * 2.5f * static_cast<float>(row++);
    }
}

void initialize_inner_rings(const float2 origin, translation_physics_movement_data * out, const float ring_radius)
{
    float angle_step = std::numbers::pi / 6;

    float distance_from_origin = ring_radius * 6;

    for (std::size_t column_it{0}; column_it < 3; ++column_it)
    {
        const float delta_size = static_cast<float>(radius_offset(generator));
        float angle{0.0f};

        for (size_t row_it{0}; row_it < 12; ++row_it)
        {
            const size_t current = row_it + column_it * 12;

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

    for (std::size_t column_it{0}; column_it < inner_cups_rows_size; ++column_it)
    {
        const float delta_size = static_cast<float>(radius_offset(generator));
        float angle{0.0f};

        for (size_t row_it{0}; row_it < inner_cups_row_size; ++row_it)
        {
            const size_t current = row_it + 36 + column_it * inner_cups_row_size;

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

    for (std::size_t job_it{0}; job_it < jobs_size; ++job_it)
    {
        handles[job_it] = scheduler.submit
        ([
        out = out + job_it * jobs_rings_size_step,
        delta_time
        ]()
        {
            OPTICK_EVENT("update_rings_position");
            for (std::size_t ring_it{0}; ring_it < jobs_rings_size_step; ++ring_it)
                out[ring_it].position = out[ring_it].position + out[ring_it].velocity * delta_time;
        });
    }

    for (std::size_t cup_it{jobs_size * jobs_rings_size_step}; cup_it < cups_size; ++cup_it)
        out[cup_it].position = out[cup_it].position + out[cup_it].velocity * delta_time;

    for (std::size_t job_it{0}; job_it < jobs_size; ++job_it)
        handles[job_it].wait();
}

void update_inner_rings_physics(translation_physics_movement_data * out, const std::size_t begin_it, const std::size_t end_it, const float delta_time)
{
    OPTICK_EVENT();
    for (std::size_t outer_it{begin_it}; outer_it < end_it; ++outer_it)
    {
        const translation_physics_movement_data outer = out[outer_it];

        for (std::size_t first_inner_it{0}; first_inner_it < inner_cups_padding; ++first_inner_it)
        {
            const std::size_t first_inner_index{first_inner_it + outer_cups_size + outer_it * inner_cups_padding};
            translation_physics_movement_data first = out[first_inner_index];

            first.position = first.position + first.velocity * delta_time;

            for (std::size_t second_inner_it{0}; second_inner_it < inner_cups_padding; ++second_inner_it)
            {
                if (first_inner_it == second_inner_it)
                    continue;

                const std::size_t second_inner_index{second_inner_it + outer_cups_size + outer_it * inner_cups_padding};
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

void update_rings(translation_physics_movement_data * out, translation_physics_movement_data * outer_cups_buffer, std::future<void> * handles, const float delta_time)
{
    OPTICK_EVENT();

    for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
    {
        out[outer_it].position = out[outer_it].position + out[outer_it].velocity * delta_time;
        outer_cups_buffer[outer_it] = out[outer_it];
    }


    std::size_t begin_it;
    std::size_t end_it{0};

    for (std::size_t job_it{0}; job_it < jobs_size; ++job_it)
    {
        begin_it = end_it;
        end_it += jobs_cups_size_step;
        handles[job_it] = scheduler.submit
        ([
        out,
        begin_it,
        end_it,
        delta_time
        ]()
        {
            update_inner_rings_physics(out, begin_it, end_it, delta_time);
        });
    }

    begin_it = end_it;
    end_it = outer_cups_size;

    update_inner_rings_physics(out, begin_it, end_it, delta_time);

    for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
    {
        translation_physics_movement_data first = outer_cups_buffer[outer_it];

        for (std::size_t inner_it{0}; inner_it < outer_cups_size; ++inner_it)
        {
            if (outer_it == inner_it)
                continue;

            translation_physics_movement_data second = outer_cups_buffer[inner_it];

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

                outer_cups_buffer[inner_it] = second;
            }

            outer_cups_buffer[outer_it] = first;
        }

    }

    OPTICK_EVENT("waiting_update");
    for (std::size_t job_it{0}; job_it < jobs_size; ++job_it)
        handles[job_it].wait();

    for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
        out[outer_it] = outer_cups_buffer[outer_it];
}
