#version 330 core
out vec4 FragColor;

in vec2 outUV;
in vec3 outFragPos;
in vec3 outNormal;

struct Material {
	sampler2D m_diffuse;
	sampler2D m_specular;

	float m_shiness;
};

uniform Material myMaterial;

struct Light {
	vec3 m_direction;
	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;
};

uniform Light myLight;

uniform sampler2D ourTexture;
uniform vec3 view_pos;

void main(){
	vec3 lDir=normalize(-myLight.m_direction); // 光照方向（取反为入射方向）

	// 环境光照强度
	vec3 _ambient = myLight.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));

	// 漫反射光照强度
	vec3 _normal = normalize(outNormal);
	//平行光的方向和入射光方向相同，因此做计算需要和之前取反（之前都是点指向光源，现在是光源指向点）
	float _diff = max(dot(_normal, -lDir), 0.0f); // 漫反射强度值（即入射角余弦值）,max防止负值（从背面射入的光）
	vec3 _diffuse = _diff * myLight.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));

	// 镜面反射光照强度
	vec3 _viewDir = normalize(view_pos - outFragPos);
	vec3 _reflectDir = reflect(lDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness); // 高光强度值，做N次方以增大高光效果
	vec3 _specular = _spec * myLight.m_specular * vec3(texture(myMaterial.m_specular, outUV));

	vec3 result = _ambient + _diffuse + _specular; // 光照总强度
	FragColor = texture(ourTexture, outUV) * vec4(result, 1.0f);
};