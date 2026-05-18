#pragma once
#include"Base.h"
#include"Shader.h"
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#define TEXTURE_DIFFUSE_STR "m_diffuse"
#define TEXTURE_SPECULAR_STR "m_specular"

namespace FF {
	// The advantage of using struct is that the memory layout of struct is continuous, which is not necessarily the case for class
	struct ffVertex {
		glm::vec3 m_pos;
		glm::vec3 m_normal;
		glm::vec2 m_texCoord;
		ffVertex() {
			m_pos = glm::vec3(0.0f);
			m_normal = glm::vec3(0.0f);
			m_texCoord = glm::vec2(0.0f);
		}
		~ffVertex() {

		}
	};

	struct ffTexture {
		uint			m_id;
		std::string		m_type;
		std::string		m_path;	
	};

	struct ffMaterial {
		glm::vec3 m_diffuseColor;
		glm::vec3 m_specularColor;
		float m_shininess;
		ffMaterial() {
			m_diffuseColor = glm::vec3(1.0f);
			m_specularColor = glm::vec3(0.0f);
			m_shininess = 32.0f;
		}
	};

	class ffMesh {
	public:
		ffMesh(std::vector<ffVertex> _vertexVec, std::vector<uint> _indexVec, std::vector<ffTexture> _texVec, ffMaterial _material);
		void draw(Shader& _shader);

	private:
		std::vector<ffVertex>	m_vertexVec;	// store vertex, nomal, UV
		std::vector<uint>		m_indexVec;		// vertex index
		std::vector<ffTexture>	m_texVec;		// store texture ID, type and path
		ffMaterial				m_material;

		GLuint m_VAO;           
		void setupMesh();
	};

	class ffModel {
	public:
		ffModel(const char* _path) {
			loadModel(_path);
		}
		void draw(Shader& _shader);
	private:
		std::vector<ffMesh>		m_meshVec;
		std::string				m_dir;

		void loadModel(std::string _path);
		void processNode(aiNode* _node, const aiScene* _scene);
		ffMesh processMesh(aiMesh* _mesh, const aiScene* _scene);
		std::vector<ffTexture> loadMaterialTextures(aiMaterial* _mat, aiTextureType _type, std::string _typeName);
	};

	class ffTextureManager {
	public:
		void SINGLE_OVER(){}
		uint creatTexture(std::string _path);
		uint creatTexture(std::string _path, std::string _dir);
	private:
		SINGLE_INSTANCE(ffTextureManager)
		ffTextureManager(){}

		std::map<std::string, uint> m_texMap; // store texture path and texID
	};
}

