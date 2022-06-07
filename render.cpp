//
// Created by GloriousSir on 6/5/2022.
//

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "profile/optick.h"

#include "render.hpp"

#include "constants.hpp"

#include "threads/BS_thread_pool.hpp"

extern BS::thread_pool scheduler;

namespace render
{
    void submit(sf::Vertex * vertices, const translation_physics_movement_data * in, std::future<void> * handles)
    {
        OPTICK_EVENT();
        static constexpr std::size_t inner_circle_vertices_size{36};
        static constexpr std::size_t outer_circle_vertices_size{144};

        handles[0] = scheduler.submit
        ([
        in,
        vertices
        ]()
        {
            OPTICK_EVENT("submit");
            for (std::size_t i{0}; i < outer_cups_size; ++i)
            {
                translation_physics_movement_data ring = in[i];

                for (std::size_t j{0}; j < outer_circle_vertices_size; j += 3)
                {
                    const std::size_t current = j + i * outer_circle_vertices_size;

                    const sf::Vector2<float> center_position = sf::Vector2<float>(ring.position.x, ring.position.y);
                    const sf::Vector2<float> position_delta = center_position - vertices[current].position;

                    vertices[current].position = vertices[current].position + position_delta;
                    vertices[current + 1].position = vertices[current + 1].position + position_delta;
                    vertices[current + 2].position = vertices[current + 2].position + position_delta;
                }
            }
        });

        for (std::size_t i{1}; i < jobs_size; ++i)
        {
            handles[i] = scheduler.submit
            ([
            vertices = vertices + outer_circle_vertices_size * outer_cups_size + (i - 1) * jobs_inner_rings_size_step * inner_circle_vertices_size,
            in = in + outer_cups_size + (i - 1) * jobs_inner_rings_size_step
            ]()
            {
                OPTICK_EVENT("submit");
                for (std::size_t i{0}; i < jobs_inner_rings_size_step; ++i)
                {
                    translation_physics_movement_data ring = in[i];

                    for (std::size_t j{0}; j < inner_circle_vertices_size; j += 3)
                    {
                        const std::size_t inner_current = j + i * inner_circle_vertices_size;

                        const sf::Vector2<float> center_position = sf::Vector2<float>(ring.position.x, ring.position.y);
                        const sf::Vector2<float> position_delta = center_position - vertices[inner_current].position;

                        vertices[inner_current].position = vertices[inner_current].position + position_delta;
                        vertices[inner_current + 1].position = vertices[inner_current + 1].position + position_delta;
                        vertices[inner_current + 2].position = vertices[inner_current + 2].position + position_delta;
                    }
                }
            });
        }

        const std::size_t inner_offset{outer_cups_size * outer_circle_vertices_size + (jobs_size - 1) * jobs_inner_rings_size_step * inner_circle_vertices_size};

        in += (jobs_size - 1) * jobs_inner_rings_size_step + outer_cups_size;
        const std::size_t remaining_inner_rings_size = inner_cups_size - (jobs_size - 1) * jobs_inner_rings_size_step;

        for (std::size_t i{0}; i < remaining_inner_rings_size; ++i)
        {
            translation_physics_movement_data ring = in[i];

            for (std::size_t j{0}; j < inner_circle_vertices_size; j += 3)
            {
                const std::size_t inner_current = j + inner_offset + i * inner_circle_vertices_size;

                const sf::Vector2<float> center_position = sf::Vector2<float>(ring.position.x, ring.position.y);
                const sf::Vector2<float> position_delta = center_position - vertices[inner_current].position;

                vertices[inner_current].position = vertices[inner_current].position + position_delta;
                vertices[inner_current + 1].position = vertices[inner_current + 1].position + position_delta;
                vertices[inner_current + 2].position = vertices[inner_current + 2].position + position_delta;
            }
        }
    }

    void render(sf::RenderWindow & window, const sf::Vertex * vertices)
    {
        OPTICK_EVENT();

        window.draw(vertices, cups_vertices_size, sf::Triangles);
    }
}