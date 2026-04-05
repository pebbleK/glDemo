#include "IO.h"
#include"ffImage.h"

namespace FF {
	ffMesh::ffMesh(std::vector<ffVertex> _vertexVec, std::vector<uint> _indexVec, std::vector<ffTexture> _texVec) {
		m_vertexVec = _vertexVec;
		m_indexVec = _indexVec;
		m_texVec = _texVec;

		setupMesh();
	}

	void ffMesh::draw(Shader& _shader) {
		uint _diffuseN = 1;
		uint _specularN = 1; 

		for (uint i = 0; i < m_texVec.size(); i++) {
			glActiveTexture(GL_TEXTURE0 + i);

			//拼装shader传参字符串
			std::string _typename = m_texVec[i].m_type;
			std::string _numStr;
			if (_typename == TEXTURE_DIFFUSE_STR) {
				_numStr = std::to_string(_diffuseN++);
			}
			if (_typename == TEXTURE_SPECULAR_STR) {
				_numStr = std::to_string(_specularN++);
			}

			_shader.setFloat("myMaterial." + _typename + _numStr, i); //这里i指的是GL_TEXTUREi
			glBindTexture(GL_TEXTURE_2D, m_texVec[i].m_id);
		}

		glActiveTexture(0);

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_indexVec.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}

	void ffMesh::setupMesh() {
		uint _VBO = 0;
		uint _EBO = 0;

		glGenVertexArrays(1, &m_VAO);
		glBindVertexArray(m_VAO);

		glGenBuffers(1, &_VBO);
		glBindBuffer(GL_ARRAY_BUFFER, _VBO);

		glBufferData(GL_ARRAY_BUFFER, sizeof(ffVertex) * m_vertexVec.size(), &m_vertexVec[0], GL_STATIC_DRAW);

		glGenBuffers(1, &_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _EBO);

		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint) * m_indexVec.size(), &m_indexVec[0], GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(ffVertex), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(ffVertex), (void*)offsetof(ffVertex, m_texCoord));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(ffVertex), (void*)offsetof(ffVertex, m_normal));

		glBindVertexArray(0);

	}

	void ffModel::loadModel(std::string _path) {
		Assimp::Importer importer;
		// 如果传入四边形，则把四边形拆分成三角形(aiProcess_Triangulate)
		// 如果纹理坐标是颠倒的，则把纹理坐标翻转(aiProcess_FlipUVs)
		const aiScene* _scene = importer.ReadFile(_path, aiProcess_Triangulate | aiProcess_FlipUVs);

		if(!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode) {
			std::cout << "model read fail!"<< std::endl;
			return;
		}

		m_dir = _path.substr(0, _path.find_last_of('/')); //获取路径中的目录部分
		processNode(_scene->mRootNode, _scene);
	}

	void ffModel::processNode(aiNode* _node, const aiScene* _scene) {
		for(uint i = 0; i < _node->mNumMeshes; i++) {
			aiMesh* _mesh = _scene->mMeshes[_node->mMeshes[i]];
			m_meshVec.push_back(processMesh(_mesh, _scene));
		}

		for (uint i = 0; i < _node->mNumChildren; i++) {
			processNode(_node->mChildren[i], _scene);
		}
	}

	ffMesh ffModel::processMesh(aiMesh* _mesh, const aiScene* _scene) {
		std::vector<ffVertex>	_vertexVec;
		std::vector<uint>		_indexVec;	
		std::vector<ffTexture>	_texVec;

		//解析顶点
		for (uint i = 0; i < _mesh->mNumVertices; i++) {
			ffVertex _vertex;

			//位置信息读取
			glm::vec3 _pos;
			_pos.x = _mesh->mVertices[i].x;
			_pos.y = _mesh->mVertices[i].y;
			_pos.z = _mesh->mVertices[i].z;
			_vertex.m_pos = _pos;

			//法线读取
			glm::vec3 _normal;
			_normal.x = _mesh->mNormals[i].x;
			_normal.y = _mesh->mNormals[i].y;
			_normal.z = _mesh->mNormals[i].z;
			_vertex.m_normal = _normal;

			//纹理坐标读取
			if (_mesh->mTextureCoords[0]) {
				glm::vec2 _texCoord;
				//纹理中有很多套，第0位置的一套UV即传统的UV坐标
				_texCoord.x = _mesh->mTextureCoords[0][i].x;
				_texCoord.y = _mesh->mTextureCoords[0][i].y;
				_vertex.m_texCoord = _texCoord;
			}

			_vertexVec.push_back(_vertex);
		}

		//解析index
		for (uint i = 0; i < _mesh->mNumFaces; i++) {
			aiFace _face = _mesh->mFaces[i];
			//每一个面mNumFaces有很多小三角形mFaces，每一个三角形有三个索引顶点
			for (uint j = 0; j < _face.mNumIndices; j++) {
				_indexVec.push_back(_face.mIndices[j]);
			}
		}

		//解析材质
		if (_mesh->mMaterialIndex >= 0) {
			aiMaterial* _mat = _scene->mMaterials[_mesh->mMaterialIndex];
			//diffuse
			std::vector<ffTexture> _diffuseVec = loadMaterialTextures(_mat,aiTextureType_DIFFUSE, TEXTURE_DIFFUSE_STR);
			_texVec.insert(_texVec.end(), _diffuseVec.begin(), _diffuseVec.end());
			//specular
			std::vector<ffTexture> _specularVec = loadMaterialTextures(_mat,aiTextureType_SPECULAR, TEXTURE_SPECULAR_STR);
			_texVec.insert(_texVec.end(), _specularVec.begin(), _specularVec.end());
		}
		
		return ffMesh(_vertexVec, _indexVec, _texVec);
	}

	std::vector<ffTexture> ffModel::loadMaterialTextures(aiMaterial* _mat, aiTextureType _type, std::string _typeName) {
		std::vector<ffTexture> _texVec;

		for (uint i = 0; i < _mat->GetTextureCount(_type); i++) {
			ffTexture _tex;

			aiString _path;
			_mat->GetTexture(_type, i, &_path); //这个纹理相对于模型的位置

			_tex.m_id = ffTextureManager::getInstance()->creatTexture(_path.C_Str(), m_dir);
			_tex.m_path = _path.C_Str();
			_tex.m_type = _typeName;

			_texVec.push_back(_tex);
		}
		return _texVec;
	}

	void ffModel::draw(Shader& _shader) {
		for(uint i = 0; i < m_meshVec.size(); i++) {
			m_meshVec[i].draw(_shader);
		}
	}

	SINGLE_INSTANCE_SET(ffTextureManager)

	uint ffTextureManager::creatTexture(std::string _path) {
		std::map<std::string, uint>::iterator _it = m_texMap.find(_path);
		if (_it != m_texMap.end()) {
			return _it->second;
		}

		ffImage* _image = ffImage::readFromFile(_path.c_str());

		uint _texID = 0;
		glGenTextures(1, &_texID);
		glBindTexture(GL_TEXTURE_2D, _texID);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, _image->getWidth(), _image->getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, _image->getData());
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete _image;
		m_texMap[_path] = _texID;
		return _texID;
	}
	uint ffTextureManager::creatTexture(std::string _path, std::string _dir) {
		return creatTexture(_dir + "/" + _path);
	}
}
