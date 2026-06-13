#pragma once
#include"Main.h"
#include"Shader.h"
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#define TEXTURE_DIFFUSE_STR "m_diffuse"
#define TEXTURE_SPECULAR_STR "m_specular"

struct Vertex {
	glm::vec3 m_pos;
	glm::vec3 m_normal;
	glm::vec2 m_texCoord;
	Vertex() {
		m_pos = glm::vec3(0.0f);
		m_normal = glm::vec3(0.0f);
		m_texCoord = glm::vec2(0.0f);
	}
	~Vertex() {

	}
};

struct Texture {
	uint			m_id;
	std::string		m_type;
	std::string		m_path;	
};

struct Material {
	glm::vec3 m_diffuseColor;
	glm::vec3 m_specularColor;
	float m_shininess;
	Material() {
		m_diffuseColor = glm::vec3(1.0f);
		// Use this specular coefficient when the MTL file does not provide Ks.
		m_specularColor = glm::vec3(0.0f);
		m_shininess = 32.0f;
	}
};

class Mesh {
public:
	Mesh(std::vector<Vertex> _vertexVec, std::vector<uint> _indexVec, std::vector<Texture> _texVec, Material _material);
	void draw(Shader& _shader);

	const std::vector<Vertex>& getVertices() const { return m_vertexVec; }
    const std::vector<uint>& getIndices() const { return m_indexVec; }

private:
	std::vector<Vertex>	m_vertexVec;	// store vertex, nomal, UV
	std::vector<uint>	m_indexVec;		// vertex index
	std::vector<Texture>	m_texVec;		// store texture ID, type and path
	Material			m_material;

	GLuint m_VAO;           
	void setupMesh();
};

class Model {
public:
	Model(const char* _path) {
		loadModel(_path);
	}
	void draw(Shader& _shader);

	const std::vector<Mesh>& getMeshes() const { return m_meshVec; }
	
private:
	std::vector<Mesh>	m_meshVec;
	std::string		m_dir;

	void loadModel(std::string _path);
	void processNode(aiNode* _node, const aiScene* _scene);
	Mesh processMesh(aiMesh* _mesh, const aiScene* _scene);
	std::vector<Texture> loadMaterialTextures(aiMaterial* _mat, aiTextureType _type, std::string _typeName);
};

class TextureManager {
public:
	void SINGLE_OVER(){}
	uint creatTexture(std::string _path);
	uint creatTexture(std::string _path, std::string _dir);
private:
	SINGLE_INSTANCE(TextureManager)
	TextureManager(){}

	std::map<std::string, uint> m_texMap; // store texture path and texID
};
