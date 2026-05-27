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
	vec3 m_pos;
	vec3 m_direction;
	float m_cutOff;
	float m_outCutOff;

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;

	float m_c;
	float m_l;
	float m_q;
};

uniform Light myLight;

uniform sampler2D ourTexture;
uniform vec3 view_pos;

void main(){
	// 边缘模糊系数
	vec3 _lightDir = normalize(myLight.m_pos - outFragPos);
	vec3 _spotDir = normalize(myLight.m_direction);
	float cosTheta = dot(-_lightDir, _spotDir);
	float epsilon = myLight.m_cutOff - myLight.m_outCutOff; //传入的值是cos值
	float intensity = clamp((cosTheta - myLight.m_outCutOff) / epsilon, 0.0f, 1.0f);

	// 强度衰减系数
	float distance = length(myLight.m_pos - outFragPos);
	float attenuation = 1.0f / (myLight.m_c + myLight.m_l * distance + myLight.m_q * (distance * distance));

	// 环境光照强度
	vec3 _ambient = myLight.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));

	// 漫反射光照强度
	vec3 _normal = normalize(outNormal);
	float _diff = max(dot(_normal, _lightDir), 0.0f); // 漫反射强度值（即入射角余弦  值）,max防止负值（从背面射入的光）
	vec3 _diffuse = _diff * myLight.m_diffuse * vec3(texture(myMaterial.m_diffuse,	outUV));

	// 镜面反射光照强度
	vec3 _viewDir = normalize(view_pos - outFragPos);
	vec3 _reflectDir = reflect(-_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness); //	高光强度值，做N次方以增大高光效果
	vec3 _specular = _spec * myLight.m_specular * vec3(texture(myMaterial.m_specular,	outUV));

	_diffuse *= intensity;
	_specular *= intensity;

	vec3 result = _ambient + _diffuse + _specular; // 光照总强度
	FragColor = texture(ourTexture, outUV) * vec4(result, 1.0f) * attenuation;
};