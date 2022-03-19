#pragma once
#include <array>
#include <glm/glm.hpp>

struct Vertex {
    glm::vec3 pos, nrm;
    glm::vec2 tex;
};

static constexpr std::array<Vertex, 6> back_vertices{
    // nz
    Vertex{
        .pos = {0.5, -0.5, -0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {-0.5, -0.5, -0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, -0.5, -0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.5, 0.5, -0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 6> front_vertices{
    // pz
    Vertex{
        .pos = {-0.5, -0.5, 0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.5, 0.5, 0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, 0.5, 0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, 0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 6> left_vertices{
    // nx
    Vertex{
        .pos = {-0.5, -0.5, -0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {-0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.5, 0.5, 0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 6> right_vertices{
    // px
    Vertex{
        .pos = {0.5, -0.5, 0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {0.5, -0.5, -0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, 0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, 0.5, 0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, -0.5, -0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, -0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 6> bottom_vertices{
    // ny
    Vertex{
        .pos = {-0.5, -0.5, -0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {0.5, -0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.5, -0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, -0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, -0.5, 0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 6> top_vertices{
    // py
    Vertex{
        .pos = {-0.5, 0.5, -0.5},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {-0.5, 0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {0.5, 0.5, -0.5},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.5, 0.5, 0.5},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.5, 0.5, 0.5},
        .tex = {1, 1},
    },
};

static constexpr std::array<Vertex, 24> cross_vertices{
    // a
    Vertex{
        .pos = {0.414, -0.5, 0.414},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {-0.414, -0.5, -0.414},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.414, 0.5, 0.414},
        .tex = {0, 1},
    },

    Vertex{
        .pos = {0.414, 0.5, 0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, -0.414},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.414, 0.5, -0.414},
        .tex = {1, 1},
    },

    // a-flipped
    Vertex{
        .pos = {0.414, -0.5, 0.414},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {0.414, 0.5, 0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, -0.414},
        .tex = {1, 0},
    },

    Vertex{
        .pos = {0.414, 0.5, 0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, 0.5, -0.414},
        .tex = {1, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, -0.414},
        .tex = {1, 0},
    },

    // b
    Vertex{
        .pos = {0.414, -0.5, -0.414},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {-0.414, -0.5, 0.414},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {0.414, 0.5, -0.414},
        .tex = {0, 1},
    },

    Vertex{
        .pos = {0.414, 0.5, -0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, 0.414},
        .tex = {1, 0},
    },
    Vertex{
        .pos = {-0.414, 0.5, 0.414},
        .tex = {1, 1},
    },

    // b-flipped
    Vertex{
        .pos = {0.414, -0.5, -0.414},
        .tex = {0, 0},
    },
    Vertex{
        .pos = {0.414, 0.5, -0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, 0.414},
        .tex = {1, 0},
    },

    Vertex{
        .pos = {0.414, 0.5, -0.414},
        .tex = {0, 1},
    },
    Vertex{
        .pos = {-0.414, 0.5, 0.414},
        .tex = {1, 1},
    },
    Vertex{
        .pos = {-0.414, -0.5, 0.414},
        .tex = {1, 0},
    },
};
