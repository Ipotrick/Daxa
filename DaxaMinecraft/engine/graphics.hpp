#pragma once
// #include <glad/glad.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <glm/glm.hpp>

auto load_texture(const char * const filepath) {
    int size_x, size_y, num_channels;
    stbi_set_flip_vertically_on_load(true);
    std::uint8_t * data = stbi_load(filepath, &size_x, &size_y, &num_channels, 0);
    // unsigned int   data_format;
    // switch (num_channels) {
    // case 1: data_format = GL_RED; break;
    // case 3: data_format = GL_RGB; break;
    // case 4: data_format = GL_RGBA; break;
    // default: data_format = GL_RGB; break;
    // }
    // GLuint tex_id;
    // glCreateTextures(GL_TEXTURE_2D, 1, &tex_id);
    // glBindTexture(GL_TEXTURE_2D, tex_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, size_x, size_y, 0, data_format, GL_UNSIGNED_BYTE, data);
    // glGenerateMipmap(GL_TEXTURE_2D);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    // return tex_id;
    return 0;
};

auto create_shader(int shader_type, const char * code_str) {
    // GLuint shader_id = glCreateShader(shader_type);
    // glShaderSource(shader_id, 1, &code_str, nullptr);
    // glCompileShader(shader_id);
    // GLint success;
    // glGetShaderiv(shader_id, GL_COMPILE_STATUS, &success);
    // if (!success) {
    //     std::string info_log;
    //     info_log.resize(512);
    //     glGetShaderInfoLog(shader_id, (GLsizei)info_log.size(), nullptr, info_log.data());
    //     std::cout << "FAILED TO COMPILE: " << info_log << "\n";
    // }
    // return shader_id;
    return 0;
};

auto load_shader(int shader_type, const char * path) {
    std::ifstream shader_file(path);
    if (!shader_file.is_open()) throw std::runtime_error("failed to open shader file");
    std::stringstream sstr;
    sstr << shader_file.rdbuf();
    return create_shader(shader_type, sstr.str().c_str());
}

auto delete_shaders(int program_id, auto... shader_ids) {
    // ((glDetachShader(program_id, shader_ids), glDeleteShader(shader_ids)), ...);
};

auto create_program(auto &&... shader_ids) {
    // GLuint program_id = glCreateProgram();
    // (glAttachShader(program_id, shader_ids), ...);
    // glLinkProgram(program_id);
    // GLint success;
    // glGetProgramiv(program_id, GL_LINK_STATUS, &success);
    // if (!success) {
    //     std::string info_log;
    //     info_log.resize(512);
    //     glGetProgramInfoLog(program_id, (GLsizei)info_log.size(), nullptr, info_log.data());
    //     std::cout << "FAILED TO LINK: " << info_log << "\n";
    // }
    // return program_id;
    return 0;
};

struct Framebuffer {
    // GLuint     fbo_id, rbo_id;
    // GLuint     pos_tex_id;
    // GLuint     nrm_tex_id;
    // GLuint     col_tex_id;
    glm::ivec2 size;

    void        bind() const {
        // glBindFramebuffer(GL_FRAMEBUFFER, fbo_id);
    }
    static void unbind() {
        // glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }
};

auto create_framebuffer(int sx, int sy) {
    Framebuffer result;

    // glGenFramebuffers(1, &result.fbo_id);
    // glBindFramebuffer(GL_FRAMEBUFFER, result.fbo_id);

    // glGenTextures(1, &result.pos_tex_id);
    // glBindTexture(GL_TEXTURE_2D, result.pos_tex_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, sx, sy, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.pos_tex_id, 0);

    // glGenTextures(1, &result.nrm_tex_id);
    // glBindTexture(GL_TEXTURE_2D, result.nrm_tex_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, sx, sy, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, result.nrm_tex_id, 0);

    // glGenTextures(1, &result.col_tex_id);
    // glBindTexture(GL_TEXTURE_2D, result.col_tex_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, sx, sy, 0, GL_RGBA, GL_FLOAT, NULL);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, result.col_tex_id, 0);

    // unsigned int attachments[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    // glDrawBuffers(3, attachments);

    // glGenTextures(1, &depth_tex_id);
    // glBindTexture(GL_TEXTURE_2D, depth_tex_id);
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, sx, sy, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
    // glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_tex_id, 0);

    // glGenRenderbuffers(1, &result.rbo_id);
    // glBindRenderbuffer(GL_RENDERBUFFER, result.rbo_id);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, sx, sy);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, result.rbo_id);

    // if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    //     std::cout << "Framebuffer is not complete!\n";

    result.size = {sx, sy};
    return result;
}

void recreate_framebuffer(int sx, int sy, Framebuffer & frame) {
    // glDeleteRenderbuffers(1, &frame.rbo_id);
    // glDeleteTextures(1, &frame.pos_tex_id);
    // glDeleteTextures(1, &frame.nrm_tex_id);
    // glDeleteTextures(1, &frame.col_tex_id);
    // glDeleteFramebuffers(1, &frame.fbo_id);
    frame = create_framebuffer(sx, sy);
}

struct GbufferQuad {
    struct Vertex {
        glm::vec2 pos;
    };

    struct Shader {
        // GLuint program_id;
        // GLint  u_g_pos_tex;
        // GLint  u_g_nrm_tex;
        // GLint  u_g_col_tex;
        // GLint  u_size;
        Shader() {
            // auto vert_shader_id = load_shader(GL_VERTEX_SHADER, "assets/frame.vert");
            // auto frag_shader_id = load_shader(GL_FRAGMENT_SHADER, "assets/frame.frag");
            // program_id          = create_program(vert_shader_id, frag_shader_id);
            // delete_shaders(program_id, vert_shader_id, frag_shader_id);
            // u_g_pos_tex = glGetUniformLocation(program_id, "g_pos_tex");
            // u_g_nrm_tex = glGetUniformLocation(program_id, "g_nrm_tex");
            // u_g_col_tex = glGetUniformLocation(program_id, "g_col_tex");
            // u_size      = glGetUniformLocation(program_id, "frame_size");
            use();
        }
        void use() {
            // glUseProgram(program_id);
            // glUniform1i(u_g_pos_tex, 0);
            // glUniform1i(u_g_nrm_tex, 1);
            // glUniform1i(u_g_col_tex, 2);
        }
    };
    static inline std::unique_ptr<Shader> shader{};

    // GLuint vao_id = -1, vbo_id = -1;

    GbufferQuad() {
        if (!shader) shader = std::make_unique<Shader>();
        // glCreateVertexArrays(1, &vao_id);
        // glBindVertexArray(vao_id);
        // glCreateBuffers(1, &vbo_id);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo_id);

        Vertex vertices[]{
            {.pos = {0, 0}}, {.pos = {1, 0}}, {.pos = {0, 1}},

            {.pos = {0, 1}}, {.pos = {1, 0}}, {.pos = {1, 1}},
        };

        // glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
        // glEnableVertexAttribArray(0);
        // glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<const void *>(0));
    }

    void draw(const Framebuffer frame) {
        // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        // glDisable(GL_CULL_FACE);
        // glDisable(GL_DEPTH_TEST);
        // glEnable(GL_CULL_FACE);
        // glEnable(GL_DEPTH_TEST);

        // glBindVertexArray(vao_id);
        // glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
        // glActiveTexture(GL_TEXTURE0);
        // glBindTexture(GL_TEXTURE_2D, frame.pos_tex_id);
        // glActiveTexture(GL_TEXTURE1);
        // glBindTexture(GL_TEXTURE_2D, frame.nrm_tex_id);
        // glActiveTexture(GL_TEXTURE2);
        // glBindTexture(GL_TEXTURE_2D, frame.col_tex_id);
        // shader->use();
        // glUniform2fv(shader->u_size, 1, (const float*)&frame.size);
        // glDrawArrays(GL_TRIANGLES, 0, 6);
    }
};
