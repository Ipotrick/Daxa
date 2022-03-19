#pragma once
#include <array>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos, nrm;
    int tex_id;
};

static constexpr std::array<Vertex, 6> back_vertices{
    // nz
    Vertex{.pos = {0.5, -0.5, -0.5}},
    Vertex{.pos = {-0.5, -0.5, -0.5}},
    Vertex{.pos = {0.5, 0.5, -0.5}},
    Vertex{.pos = {0.5, 0.5, -0.5}},
    Vertex{.pos = {-0.5, -0.5, -0.5}},
    Vertex{.pos = {-0.5, 0.5, -0.5}},
};

static constexpr std::array<Vertex, 6> front_vertices{
    // pz
    Vertex{.pos = {-0.5, -0.5, 0.5}},
    Vertex{.pos = {0.5, -0.5, 0.5}},
    Vertex{.pos = {-0.5, 0.5, 0.5}},
    Vertex{.pos = {-0.5, 0.5, 0.5}},
    Vertex{.pos = {0.5, -0.5, 0.5}},
    Vertex{.pos = {0.5, 0.5, 0.5}},
};

static constexpr std::array<Vertex, 6> left_vertices{
    // nx
    Vertex{.pos = {-0.5, -0.5, -0.5}},
    Vertex{.pos = {-0.5, -0.5, 0.5}},
    Vertex{.pos = {-0.5, 0.5, -0.5}},
    Vertex{.pos = {-0.5, 0.5, -0.5}},
    Vertex{.pos = {-0.5, -0.5, 0.5}},
    Vertex{.pos = {-0.5, 0.5, 0.5}},
};

static constexpr std::array<Vertex, 6> right_vertices{
    // px
    Vertex{.pos = {0.5, -0.5, 0.5}},
    Vertex{.pos = {0.5, -0.5, -0.5}},
    Vertex{.pos = {0.5, 0.5, 0.5}},
    Vertex{.pos = {0.5, 0.5, 0.5}},
    Vertex{.pos = {0.5, -0.5, -0.5}},
    Vertex{.pos = {0.5, 0.5, -0.5}},
};

static constexpr std::array<Vertex, 6> bottom_vertices{
    // ny
    Vertex{.pos = {-0.5, -0.5, -0.5}},
    Vertex{.pos = {0.5, -0.5, -0.5}},
    Vertex{.pos = {-0.5, -0.5, 0.5}},
    Vertex{.pos = {-0.5, -0.5, 0.5}},
    Vertex{.pos = {0.5, -0.5, -0.5}},
    Vertex{.pos = {0.5, -0.5, 0.5}},
};

static constexpr std::array<Vertex, 6> top_vertices{
    // py
    Vertex{.pos = {-0.5, 0.5, -0.5}},
    Vertex{.pos = {-0.5, 0.5, 0.5}},
    Vertex{.pos = {0.5, 0.5, -0.5}},
    Vertex{.pos = {0.5, 0.5, -0.5}},
    Vertex{.pos = {-0.5, 0.5, 0.5}},
    Vertex{.pos = {0.5, 0.5, 0.5}},
};

static constexpr float root2 = 0.414f;

static constexpr std::array<Vertex, 24> cross_vertices{
    // clang-format off
    
    // a
    Vertex{.pos = { root2, -0.5,  root2}},
    Vertex{.pos = {-root2, -0.5, -root2}},
    Vertex{.pos = { root2,  0.5,  root2}},
    Vertex{.pos = { root2,  0.5,  root2}},
    Vertex{.pos = {-root2, -0.5, -root2}},
    Vertex{.pos = {-root2,  0.5, -root2}},

    // a-flipped
    Vertex{.pos = {-root2, -0.5, -root2}},
    Vertex{.pos = { root2, -0.5,  root2}},
    Vertex{.pos = {-root2,  0.5, -root2}},
    Vertex{.pos = {-root2,  0.5, -root2}},
    Vertex{.pos = { root2, -0.5,  root2}},
    Vertex{.pos = { root2,  0.5,  root2}},

    // b
    Vertex{.pos = { root2, -0.5, -root2}},
    Vertex{.pos = {-root2, -0.5,  root2}},
    Vertex{.pos = { root2,  0.5, -root2}},
    Vertex{.pos = { root2,  0.5, -root2}},
    Vertex{.pos = {-root2, -0.5,  root2}},
    Vertex{.pos = {-root2,  0.5,  root2}},

    // b-flipped
    Vertex{.pos = {-root2, -0.5,  root2}},
    Vertex{.pos = { root2, -0.5, -root2}},
    Vertex{.pos = {-root2,  0.5,  root2}},
    Vertex{.pos = {-root2,  0.5,  root2}},
    Vertex{.pos = { root2, -0.5, -root2}},
    Vertex{.pos = { root2,  0.5, -root2}},

    // clang-format on
};
