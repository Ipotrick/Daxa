#pragma once
#include <engine/graphics.hpp>
#include <engine/world/chunk.hpp>

struct RenderableChunk {
    struct Shader {
        // GLuint program_id;
        // GLint  u_mvp_mat, u_tex0;
        Shader() {
            // auto vert_shader_id = load_shader(GL_VERTEX_SHADER, "assets/chunk.vert");
            // auto frag_shader_id = load_shader(GL_FRAGMENT_SHADER, "assets/chunk.frag");
            // program_id          = create_program(vert_shader_id, frag_shader_id);
            // delete_shaders(program_id, vert_shader_id, frag_shader_id);
            // u_mvp_mat = glGetUniformLocation(program_id, "mvp_mat");
            // u_tex0    = glGetUniformLocation(program_id, "tex");
        }
        void use(const float * mvp_mat_ptr) {
            // glUseProgram(program_id);
            // glUniformMatrix4fv(u_mvp_mat, 1, GL_FALSE, mvp_mat_ptr);
            // glUniform1i(u_tex0, 0);
        }
    };
    struct Textures {
        // GLuint id0;
        Textures() {
            // id0 = load_texture("assets/atlas.png");
        }
    };
    static inline std::unique_ptr<Shader>   shader{};
    static inline std::unique_ptr<Textures> textures{};

    // GLuint    vao_id = -1, vbo_id = -1;
    Chunk     chunk;
    glm::mat4 modl_mat = glm::translate(glm::mat4(1), Chunk::FLOAT_DIM * glm::vec3(chunk.pos));

    RenderableChunk(glm::ivec3 p) : chunk{.pos = p} {
        if (!shader) shader = std::make_unique<Shader>();
        if (!textures) textures = std::make_unique<Textures>();
        // glCreateVertexArrays(1, &vao_id);
        // glBindVertexArray(vao_id);
        // glCreateBuffers(1, &vbo_id);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        // glBufferData(GL_ARRAY_BUFFER, Chunk::MAX_SIZE, nullptr, GL_DYNAMIC_DRAW);
        size_t off = 0;
        
        // ENABLE_ATTRIB(0, Vertex, pos);
        // ENABLE_ATTRIB(1, Vertex, nrm);
        // ENABLE_ATTRIB(2, Vertex, tex);

        chunk.generate_block_data();
    }

    void draw(const glm::mat4 & viewproj_mat) {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glDisable(GL_CULL_FACE);
        // glEnable(GL_CULL_FACE);
        // glEnable(GL_DEPTH_TEST);

        // glBindVertexArray(vao_id);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

        if (chunk.needs_remesh) {
            // auto * mapped_vertices = static_cast<Vertex *>(glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY));
            // chunk.generate_mesh_data(mapped_vertices);
            // glUnmapBuffer(GL_ARRAY_BUFFER);
        }

        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, textures->id0);
        glm::mat4 mvp_mat = viewproj_mat * modl_mat;
        // shader->use(reinterpret_cast<const GLfloat *>(&mvp_mat));
        // glDrawArrays(GL_TRIANGLES, 0, chunk.vert_n);
    }
};

// static constexpr auto size = sizeof(RenderableChunk);
