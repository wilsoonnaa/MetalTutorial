//
//  mesh.cpp
//  Metal-Tutorial
//

#include "mesh.hpp"

#include <iostream>
#include <unordered_map>
#include <string>

Mesh::Mesh(std::string filePath, MTL::Device* metalDevice) {
    tinyobj::attrib_t vertexArrays;
    std::vector<tinyobj::shape_t>shapes;
    std::vector<tinyobj::material_t> materials;
    
    std::string baseDirectory = filePath.substr(0, filePath.find_last_of("/\\") + 1); // Base directory for the .mtl file
    std::string warning;
    std::string error;
    
    tinyobj::LoadObj(&vertexArrays, &shapes, &materials, &error, filePath.c_str(), baseDirectory.c_str());
    if (!error.empty())
        std::cout << "TINYOBJ::ERROR: " << error << std::endl;
    
    
    std::unordered_map<std::string, int> diffuseTextureIndexMap;
    int count = 0;
    // Load Textures
    std::vector<std::string> diffuseFilePaths;
    std::cout << "Loading Textures..." << std::endl;
    for(int i = 0; i < materials.size(); i++) {
        if (!materials[i].diffuse_texname.empty()) {
            std::cout << count+1 << ".) " << baseDirectory + materials[i].diffuse_texname << std::endl;
            diffuseFilePaths.push_back(baseDirectory + materials[i].diffuse_texname);
            diffuseTextureIndexMap[materials[i].diffuse_texname] = count++;
        }
    }
    textures = new TextureArray(diffuseFilePaths,
                                metalDevice);
    
    // Loop over Shapes
    for (int s = 0; s < shapes.size(); s++) {
        // Loop over Faces (polygon)
        int index_offset = 0;
        for (int f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            // Get the diffuse texture name for a particular face
            int material_id = shapes[s].mesh.material_ids[f];
            std::string diffuseTextureName = materials[material_id].diffuse_texname;
            // Hardcode loading triangles
            int fv = 3;
            // Loop over vertices in the face
            for (int v = 0; v < fv; v++) {
                // Access to Vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                
                Vertex vertex{};
                // Vertex position
                vertex.position = {
                    vertexArrays.vertices[3 * idx.vertex_index + 0],
                    vertexArrays.vertices[3 * idx.vertex_index + 1],
                    vertexArrays.vertices[3 * idx.vertex_index + 2],
                };
                // Vertex Normal
                vertex.normal = {
                    vertexArrays.normals[3 * idx.normal_index + 0],
                    vertexArrays.normals[3 * idx.normal_index + 1],
                    vertexArrays.normals[3 * idx.normal_index + 2]
                };
                // Vertex Texture Coordinates
                vertex.textureCoordinate = {
                    vertexArrays.texcoords[2 * idx.texcoord_index + 0],
                    vertexArrays.texcoords[2 * idx.texcoord_index + 1]
                };
                // Texture Indices
                vertex.diffuseTextureIndex = {
                    diffuseTextureIndexMap[diffuseTextureName]
                };
                // Vertex Indices
                if (vertexMap.count(vertex) == 0) {
                    vertexMap[vertex] = (uint32_t)vertices.size();
                    vertices.push_back(vertex);
                }
                
                vertexIndices.push_back(vertexMap[vertex]);
            }
            index_offset += fv;
        }
    }
}
