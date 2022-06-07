//
// Created by GloriousSir on 6/6/2022.
//

#ifndef SWEEPING_PROCESS_CONSTANTS_HPP
#define SWEEPING_PROCESS_CONSTANTS_HPP

#include <cstdint>
#include <cstddef>

constexpr float outer_cup_radius{525};

constexpr std::size_t inner_cups_rows_size{20};
constexpr std::size_t inner_cups_row_size{24};

constexpr std::size_t outer_cups_size{90};
constexpr std::size_t inner_cups_padding{36 + inner_cups_row_size * inner_cups_rows_size};
constexpr std::size_t inner_cups_size{outer_cups_size * inner_cups_padding};

constexpr std::size_t cups_size{inner_cups_size + outer_cups_size};

constexpr std::size_t cups_vertices_size{outer_cups_size * 144 + inner_cups_size * 36};

constexpr std::size_t jobs_size{14};
constexpr std::size_t jobs_cups_size_step{(outer_cups_size * (jobs_size - 1) / jobs_size + ((outer_cups_size * (jobs_size - 1) / jobs_size) % jobs_size)) / jobs_size};
constexpr std::size_t jobs_rings_size_step{cups_size * (jobs_size - 1) / jobs_size / jobs_size};
constexpr std::size_t jobs_inner_rings_size_step{inner_cups_size * (jobs_size - 1) / jobs_size / jobs_size};


#endif //SWEEPING_PROCESS_CONSTANTS_HPP