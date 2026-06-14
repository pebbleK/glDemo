#include "Collision.h"

void TerrainCollider::buildFromModel(const Model &model, const glm::mat4 modelMatrix){
    m_vertices.clear();
    m_indices.clear();

    for(const Mesh &mesh : model.getMeshes()){
        uint baseIndex = static_cast<uint>(m_vertices.size());

        for (const Vertex &vertex : mesh.getVertices()) {
            glm::vec4 worldPos = modelMatrix * glm::vec4(vertex.m_pos, 1.0f);
            m_vertices.push_back(glm::vec3(worldPos));
        }

        for (uint index : mesh.getIndices()) {
            m_indices.push_back(baseIndex + index);
        }
    }
    
}

bool TerrainCollider::getHeightAt(float x, float z, float &outY) const{
    glm::vec2 p(x, z);
    bool found  = false;
    float bestY = -FLT_MAX;

    for(size_t i = 0; i + 2 < m_indices.size(); i += 3){
        glm::vec3 a = m_vertices[m_indices[i]];
        glm::vec3 b = m_vertices[m_indices[i+1]];
        glm::vec3 c = m_vertices[m_indices[i+2]];

        glm::vec2 a2(a.x, a.z);
        glm::vec2 b2(b.x, b.z);
        glm::vec2 c2(c.x, c.z);

        float denom = (b2.y - c2.y) * (a2.x - c2.x)
            + (c2.x - b2.x) * (a2.y - c2.y);

        if(abs(denom) < 0.00001f){
            continue;
        }

        float w1 = ((b2.y - c2.y) * (p.x - c2.x) 
            + (c2.x - b2.x) * (p.y - c2.y)) / denom;

        float w2 = ((c2.y - a2.y) * (p.x - c2.x)
        + (a2.x - c2.x) * (p.y - c2.y)) / denom;

        float w3 = 1.0f - w1 - w2;

        if (w1 >= 0.0f && w2 >= 0.0f && w3 >= 0.0f) {
            float y = w1 * a.y + w2 * b.y + w3 * c.y;

            if (y > bestY) {
                bestY = y;
                found = true;
            }
        }
    }

    if (found) {
        outY = bestY;
    }

    return found;
}