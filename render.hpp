//
// Created by GloriousSir on 6/5/2022.
//

#ifndef SWEEPING_PROCESS_RENDER_HPP
#define SWEEPING_PROCESS_RENDER_HPP

#include <future>

#include "process.hpp"

namespace render
{
    void submit(sf::Vertex *, const translation_physics_movement_data *, std::future<void> *);
    void render(sf::RenderWindow &, const sf::Vertex *);
}


#endif //SWEEPING_PROCESS_RENDER_HPP
