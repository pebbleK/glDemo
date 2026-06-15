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
    /*
    三角形重心坐标插值算法：
        空间内三角形投影到xz平面三角形abc，分别计算pab, pbc, pca三角形的有向二倍面积值，
        计算denom先规定了三个点的顺序，然后判断pab, pbc, pca的3个有向值与denom同号，则说明p点在abc三角形内。
        同时这3个三角形计算出的权重w1, w2, w3可以作为p距离c, a, b三个点的权重，值越大说明越接近那个点。
    */ 
    glm::vec2 p(x, z);
    bool found  = false;
    float bestY = -FLT_MAX;

    for(size_t i = 0; i + 2 < m_indices.size(); i += 3){
        glm::vec3 a = m_vertices[m_indices[i]];
        glm::vec3 b = m_vertices[m_indices[i+1]];
        glm::vec3 c = m_vertices[m_indices[i+2]];

        // 空间投影到xz平面的三角形
        glm::vec2 a2(a.x, a.z);
        glm::vec2 b2(b.x, b.z);
        glm::vec2 c2(c.x, c.z);

        float denom = (b2.y - c2.y) * (a2.x - c2.x)
            + (c2.x - b2.x) * (a2.y - c2.y);

        // 判断有向二倍面积值是否为0，为0说明三个点共线
        if(abs(denom) < 0.00001f){
            continue;
        }

        float w1 = ((b2.y - c2.y) * (p.x - c2.x) 
            + (c2.x - b2.x) * (p.y - c2.y)) / denom;

        float w2 = ((c2.y - a2.y) * (p.x - c2.x)
        + (a2.x - c2.x) * (p.y - c2.y)) / denom;

        float w3 = 1.0f - w1 - w2;

        // 判断拆分的3个三角形w值是否与二倍有向面积值同号
        if (w1 >= 0.0f && w2 >= 0.0f && w3 >= 0.0f) {
            float y = w1 * a.y + w2 * b.y + w3 * c.y;

            // 根据权重计算空间中重心y
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