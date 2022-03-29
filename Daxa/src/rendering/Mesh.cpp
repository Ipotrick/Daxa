#include "Mesh.hpp"

#include <fstream>

#include <cgltf.h>

namespace daxa {
    std::optional<Mesh> Mesh::tryLoadFromGLTF2(std::filesystem::path path) {
        
        std::ifstream ifs{path};
        if (!ifs) {
            printf("invalid file path\n");
            return std::nullopt;
        }
        std::string buffer;
        for( std::string line; std::getline( ifs, line ); ){
            buffer.append(line);
        }

        cgltf_options options = {};
        cgltf_data* data = NULL; 
        cgltf_result result = cgltf_parse(&options, buffer.data(), buffer.size(), &data);
        if (result == cgltf_result_success)
        {
            printf("buffer_count: %llu\n",data->buffers_count);
            for (int i = 0; i < data->buffers_count; i++) {
                printf("buffer %i has a size of %llu bytes: \n", i, data->buffers[i].size);
            }
            printf("scenes_count: %llu\n", data->scenes_count); 
            for (int i = 0; i < data->scenes_count; i++) {
                printf("scene %i has the name: %s\n",i,data->scenes[i].name);
                
            }
            
            /* TODO make awesome stuff */
            cgltf_free(data);
            return std::optional{ Mesh{} };
        } else {
            printf("failed to parse gltf\n");
            return std::nullopt;
        }
    }
}