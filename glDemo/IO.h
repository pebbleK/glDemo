#pragma once
#include"Base.h"
#include"Shader.h"
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#define TEXTURE_DIFFUSE_STR "texture_diffuse"
#define TEXTURE_SPECULAR_STR "texture_specular"

namespace FF {
	//使用struct的好处就是struct的内存排布是连续的，class不一定
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

	class ffMesh {
	public:
		ffMesh(std::vector<ffVertex> _vertexVec, std::vector<uint> _indexVec, std::vector<ffTexture> _texVec);
		void draw(Shader& _shader);

	private:
		std::vector<ffVertex>	m_vertexVec;	//存储顶点，法线，UV
		std::vector<uint>		m_indexVec;		//顶点索引
		std::vector<ffTexture>	m_texVec;		//存储纹理id，类型，路径

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

		std::map<std::string, uint> m_texMap; //存储纹理路径和texID
	};
}

