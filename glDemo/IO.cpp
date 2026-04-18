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
		uint diffuseTex = 0;
		uint specularTex = 0;
		for (uint i = 0; i < m_texVec.size(); i++) {
			if (m_texVec[i].m_type == TEXTURE_DIFFUSE_STR && diffuseTex == 0) {
				diffuseTex = m_texVec[i].m_id;
			}
			else if (m_texVec[i].m_type == TEXTURE_SPECULAR_STR && specularTex == 0) {
				specularTex = m_texVec[i].m_id;
			}
		}
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, diffuseTex);
		_shader.setInt("myMaterial.m_diffuse", 0);

		if (specularTex != 0) {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, specularTex);
			_shader.setInt("myMaterial.m_specular", 1);
			_shader.setInt("myMaterial.hasSpecularMap", 1);
		}
		else {
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);
			_shader.setInt("myMaterial.hasSpecularMap", 0);
		}

		glBindVertexArray(m_VAO);
		glDrawElements(GL_TRIANGLES, m_indexVec.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
		glActiveTexture(GL_TEXTURE0);
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
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(ffVertex), (void*)offsetof(ffVertex, m_texCoord));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(ffVertex), (void*)offsetof(ffVertex, m_normal));

		glBindVertexArray(0);

	}

	void ffModel::loadModel(std::string _path) {
		Assimp::Importer importer;
		// If a quadrilateral is passed, split it into triangles (aiProcess_Triangulate)
		// If the texture coordinates are inverted, flip the texture coordinates (aiProcess_FlipUVs)
		const aiScene* _scene = importer.ReadFile(_path, aiProcess_Triangulate);
		
		if(!_scene || _scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !_scene->mRootNode) {
			std::cout << "model read fail!"<< std::endl;
			return;
		}

		m_dir = _path.substr(0, _path.find_last_of('/')); //Get the directory part of the path
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

		//Parse vertices
		for (uint i = 0; i < _mesh->mNumVertices; i++) {
			ffVertex _vertex;

			//Location information reading
			glm::vec3 _pos;
			_pos.x = _mesh->mVertices[i].x;
			_pos.y = _mesh->mVertices[i].y;
			_pos.z = _mesh->mVertices[i].z;
			_vertex.m_pos = _pos;

			//normal reading
			glm::vec3 _normal;
			_normal.x = _mesh->mNormals[i].x;
			_normal.y = _mesh->mNormals[i].y;
			_normal.z = _mesh->mNormals[i].z;
			_vertex.m_normal = _normal;

			//texture coordinate reading
			if (_mesh->mTextureCoords[0]) {
				glm::vec2 _texCoord;
				//There are many sets in the texture, and the UV set at position 0 is the traditional UV coordinates.
				_texCoord.x = _mesh->mTextureCoords[0][i].x;
				_texCoord.y = _mesh->mTextureCoords[0][i].y;
				_vertex.m_texCoord = _texCoord;
			}

			_vertexVec.push_back(_vertex);
		}

		//Parse index
		for (uint i = 0; i < _mesh->mNumFaces; i++) {
			aiFace _face = _mesh->mFaces[i];
			//Each face mNumFaces contains many small triangles mFaces, and each triangle has three vertex indices.
			for (uint j = 0; j < _face.mNumIndices; j++) {
				_indexVec.push_back(_face.mIndices[j]);
			}
		}

		//Parse texture
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
			_mat->GetTexture(_type, i, &_path); //The position of this texture relative to the model

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
