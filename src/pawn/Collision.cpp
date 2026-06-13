#include "Collision.h"

void TerrainCollider::buildFromModel(const Model &model, const glm::mat4 modelMatrix){
    m_vertices.clear();
    m_indices.clear();

    for(const Mesh &mesh : model.getMeshes()){
        uint baseIndex = static_cast<uint>(m_vertices.size());

        for (const Vertex& vertex : mesh.getVertices()) {
            glm::vec4 worldPos = modelMatrix * glm::vec4(vertex.m_pos, 1.0f);
            m_vertices.push_back(glm::vec3(worldPos));
        }

        for (uint index : mesh.getIndices()) {
            m_indices.push_back(baseIndex + index);
        }
    }
    
}