//
// Created by GloriousSir on 1/20/2022.
//

#include <numbers>
#include <iostream>
#include "shape.hpp"

#include "constants.hpp"


void make_graphical_vertex_rings(const translation_physics_movement_data * in, sf::Vertex * out)
{
    constexpr std::size_t outer_vertices_size{144};
    constexpr std::size_t outer_segments_size{outer_vertices_size / 3};

    const math::float2 position = in[0].position;
    const float radius = in[0].radius;

    float angle = 0.0f;
    const float outer_angle_step = 2 * std::numbers::pi / outer_segments_size;

    std::size_t current{};

    for (std::size_t i{0}; i < outer_cups_size; ++i)
    {
        for (std::size_t j{0}; j < outer_vertices_size; j += 3)
        {
            current = j + i * outer_vertices_size;

            sf::Vertex vertex({position.x, position.y}, sf::Color::Black);
            out[current] = vertex;

            math::float2 new_position = position + math::float2{cosf(angle) * radius, sinf(angle) * radius};
            vertex = sf::Vertex({new_position.x, new_position.y}, sf::Color::Black);
            out[current + 1] = vertex;

            angle += outer_angle_step;

            new_position = position + math::float2{cosf(angle) * radius, sinf(angle) * radius};
            vertex = sf::Vertex({new_position.x, new_position.y}, sf::Color::Black);
            out[current + 2] = vertex;
        }
    }
    constexpr std::size_t inner_vertices_size{36};
    constexpr std::size_t inner_segments_size{inner_vertices_size / 3};
    current += 3;
    const float inner_angle_step = 2 * std::numbers::pi / inner_segments_size;

    for (std::size_t i{0}; i < inner_cups_size; ++i)
    {
        const math::float2 inner_position = in[i + outer_cups_size].position;
        const float inner_radius = in[i + outer_cups_size].radius;

        angle = 0.0f;

        for (std::size_t j{0}; j < inner_vertices_size; j += 3)
        {
            const std::size_t inner_current = j + current + i * inner_vertices_size;

            sf::Vertex vertex({inner_position.x, inner_position.y}, sf::Color::Red);
            out[inner_current] = vertex;

            math::float2 new_position = inner_position + math::float2{cosf(angle) * inner_radius, sinf(angle) * inner_radius};
            vertex = sf::Vertex({new_position.x, new_position.y}, sf::Color::Red);
            out[inner_current + 1] = vertex;

            angle += inner_angle_step;

            new_position = inner_position + math::float2{cosf(angle) * inner_radius, sinf(angle) * inner_radius};
            vertex = sf::Vertex({new_position.x, new_position.y}, sf::Color::Red);
            out[inner_current + 2] = vertex;
        }
    }
}
