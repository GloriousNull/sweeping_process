#include <numbers>
#include "SFML/Graphics.hpp"

#include "ui.hpp"

#include "render.hpp"
#include "process.hpp"
#include "shape.hpp"

#include "constants.hpp"

#include "threads/BS_thread_pool.hpp"

#include "profile/optick.h"

BS::thread_pool scheduler{};

int main()
{
    ui::SetCurrentContext(ui::CreateContext());
    auto & io = ImGui::GetIO();
    io.MouseDrawCursor = true;

    sf::ContextSettings settings{};
    settings.antialiasingLevel = 8;

    constexpr uint32_t window_width{screen_size.x}, window_height{screen_size.y};
    sf::RenderWindow window(sf::VideoMode(window_width, window_height), "Sweeping process", sf::Style::Default, settings);
    window.setFramerateLimit(144);
    window.setMouseCursorVisible(true);

    if (!ui::SFML::Init(window))
        return EXIT_FAILURE;

    std::future<void> * handles = new std::future<void>[jobs_size];

    translation_physics_movement_data * cups = new translation_physics_movement_data[cups_size];
    translation_physics_movement_data * outer_cups_buffer = new translation_physics_movement_data[outer_cups_size];

    initialize_outer_rings({window_width * 0.5f, window_height * 0.5f}, cups);
    for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
    {
        const std::size_t offset{outer_cups_size + outer_it * inner_cups_padding};

        initialize_inner_rings(cups[outer_it].position, cups + offset, 8.0f);
    }

    // graphics
    sf::Vertex * rings_vertices = new sf::Vertex[cups_vertices_size];
    make_graphical_vertex_rings(cups, rings_vertices);

    math::float2 camera_vector{0, 0};
    constexpr float camera_speed{30};

    bool active{false};

    render::submit(rings_vertices, cups, handles);

    sf::Clock delta_clock;
    while (window.isOpen())
    {
        OPTICK_FRAME("MainThread");

        sf::Event event{};

        while (window.pollEvent(event))
        {
            ui::SFML::ProcessEvent(window, event);

            switch (event.type)
            {
                case sf::Event::Closed:
                {
                    window.close();

                    break;
                }

                case sf::Event::MouseWheelMoved:
                {
                    auto view = window.getView();
                    const float zoom = 1.0f + -0.25f * static_cast<float>(event.mouseWheel.delta);
                    view.zoom(zoom);

                    window.setView(view);

                    break;
                }

                case sf::Event::KeyPressed:
                {
                    switch(event.key.code)
                    {
                        case sf::Keyboard::Down:
                        {
                            auto view = window.getView();
                            const float zoom = 1.0f + 0.25f;
                            view.zoom(zoom);

                            window.setView(view);

                            break;
                        }
                        case sf::Keyboard::Up:
                        {
                            auto view = window.getView();
                            const float zoom = 1.0f + -0.25f;
                            view.zoom(zoom);

                            window.setView(view);

                            break;
                        }
                        case sf::Keyboard::K:
                        {
                            active = true;

                            break;
                        }

                        case sf::Keyboard::P:
                        {
                            active = false;

                            break;
                        }

                        case sf::Keyboard::R:
                        {
                            active = false;

                            initialize_outer_rings({window_width * 0.5f, window_height * 0.5f}, cups);
                            for (std::size_t i{0}; i < outer_cups_size; ++i)
                            {
                                const std::size_t offset{outer_cups_size + i * inner_cups_padding};

                                initialize_inner_rings(cups[i].position, cups + offset, 8.0f);
                            }

                            make_graphical_vertex_rings(cups, rings_vertices);

                            render::submit(rings_vertices, cups, handles);

                            break;
                        }

                        case sf::Keyboard::W:
                        {
                            camera_vector.y = -1;

                            break;
                        }

                        case sf::Keyboard::S:
                        {
                            camera_vector.y = 1;

                            break;
                        }

                        case sf::Keyboard::A:
                        {
                            camera_vector.x = -1;

                            break;
                        }

                        case sf::Keyboard::D:
                        {
                            camera_vector.x = 1;
                        }

                        default: {}
                    }

                    break;
                }

                case sf::Event::KeyReleased:
                {
                    switch(event.key.code)
                    {
                        case sf::Keyboard::W:
                        case sf::Keyboard::S:
                        {
                            camera_vector.y = 0;

                            break;
                        }

                        case sf::Keyboard::A:
                        case sf::Keyboard::D:
                        {
                            camera_vector.x = 0;
                        }

                        default: {}
                    }
                }

                default: {}
            }
        }

        const float delta_time = delta_clock.getElapsedTime().asSeconds();

        if (active)
        {
//            update_rings_position(cups, delta_time, handles);

            update_rings(cups, outer_cups_buffer, handles, delta_time);

            render::submit(rings_vertices, cups, handles);
        }

        const auto render_delay = delta_clock.getElapsedTime();
        ui::SFML::Update(window, delta_clock.restart());

        constexpr auto ui_settings_bar = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove;

        auto view = window.getView();

        auto previous_camera_coordinates = view.getCenter();
        const math::float2 camera_delta_vector = camera_vector * camera_speed;
        view.setCenter(previous_camera_coordinates.x + camera_delta_vector.x, previous_camera_coordinates.y + camera_delta_vector.y);
        window.setView(view);

        {
            OPTICK_EVENT("ui")
            ui::Begin("stats", nullptr, ui_settings_bar | ImGuiWindowFlags_NoDecoration);
            if (ui::Button("start"))
                active = true;
            if (ui::Button("stop"))
                active = false;
            if (ui::Button("reset"))
            {
                active = false;
                initialize_outer_rings({window_width * 0.5f, window_height * 0.5f}, cups);
                for (std::size_t outer_it{0}; outer_it < outer_cups_size; ++outer_it)
                {
                    const std::size_t offset{outer_cups_size + outer_it * inner_cups_padding};

                    initialize_inner_rings(cups[outer_it].position, cups + offset, 8.0f);
                }

                make_graphical_vertex_rings(cups, rings_vertices);
                render::submit(rings_vertices, cups, handles);
            }
            ui::SetWindowPos({0, window_height-ui::GetWindowHeight()}, true);
            ui::SetWindowFontScale(2.0f);
            ui::Text("render delay: %d ml", render_delay.asMilliseconds());
            ui::Text("fps: %f", 1 / render_delay.asSeconds());
            ui::End();

            ui::Begin(" ", nullptr, ui_settings_bar | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoDecoration);
            ui::SetWindowPos({window_width-ui::GetWindowWidth(), 0}, true);
            ui::SetWindowFontScale(3.0f);
            if (ui::Button("exit"))
                window.close();
            ui::End();
        }

        window.clear(sf::Color::Magenta);

        {
            OPTICK_EVENT("waiting_submit")
            for (std::size_t job_it{0}; job_it < jobs_size; ++job_it)
                handles[job_it].wait();
        }

        render::render(window, rings_vertices);

        ui::SFML::Render(window);
        window.display();
    }

    delete[] rings_vertices;
    delete[] handles;
    delete[] outer_cups_buffer;
    delete[] cups;
    ui::SFML::Shutdown();

    return EXIT_SUCCESS;
}
