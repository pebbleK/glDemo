#pragma once

#include "Main.h"
#include "read.h"

class TerrainCollider{
public:
    void buildFromModel(const Model &model, const glm::mat4 modelMatrix);
    bool getHeightAt(float x, float &outY, float z) const;

private:
    std::vector<glm::vec3> m_vertices;
    std::vector<uint> m_indices;
};