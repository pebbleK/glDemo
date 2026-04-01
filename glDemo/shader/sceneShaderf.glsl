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

// 平行光光源结构体
struct DirLight {
	vec3 m_direction; // 光源方向

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;
};

// 点光光源结构体
struct PointLight {
	vec3 m_pos;

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;

	float m_c;
	float m_l;
	float m_q;
};

// 聚光灯光源结构体
struct SpotLight {
	vec3 m_pos;
	vec3 m_direction;

	float m_cutOff;
	float m_outerCutOff;

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;

	float m_c;
	float m_l;
	float m_q;
};

//#define MAX_POINT_NUMBER 4
uniform DirLight _dirLight;
uniform PointLight _pointLight;
uniform SpotLight _spotLight;

uniform vec3 view_pos;

vec3 calculateDir(DirLight _light, vec3 _normal, vec3 _viewDir){
	// 归一化光线方向
	vec3 _lightDir = normalize(_light.m_direction); // 平行光光源方向是射入方向

	// 环境光照强度
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));

	// 漫反射光照强度
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));

	// 镜面反射光照强度
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * vec3(texture(myMaterial.m_specular, outUV));

	return (_ambient + _diffuse + _specular);
}

vec3 calculatePoint(PointLight _light, vec3 _normal, vec3 _viewDir, vec3 _fragPos){
	vec3 _lightDir = normalize(_fragPos - _light.m_pos);
	// 环境光照强度
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));
	// 漫反射光照强度
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));
	// 镜面反射光照强度
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * vec3(texture(myMaterial.m_specular, outUV));
	
	// 距离衰减
	float _dis = length(_light.m_pos - _fragPos);
	float _attenuation = 1.0f / (_light.m_c + _light.m_l * _dis + _light.m_q * (_dis * _dis));

	return (_ambient + _diffuse + _specular) * _attenuation;
}

vec3 calculateSpot(SpotLight _light, vec3 _normal, vec3 _viewDir, vec3 _fragPos){
	vec3 _lightDir = normalize(_fragPos - _light.m_pos);
	// 环境光照强度
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));
	// 漫反射光照强度
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));
	// 镜面反射光照强度
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * vec3(texture(myMaterial.m_specular, outUV));
	
	// 距离衰减
	float _dis = length(_light.m_pos - _fragPos);
	float _attenuation = 1.0f / (_light.m_c + _light.m_l * _dis + _light.m_q * (_dis * _dis));
	// 聚光衰减
	vec3 _spotDir = normalize(_light.m_direction); // 聚光灯指向方向
	float _cosTheta = dot(_lightDir, _spotDir); // 点方向光线与聚光灯方向的夹角余弦
	float _epsilon = _light.m_cutOff - _light.m_outerCutOff; // 模糊边缘的角度余弦
	float _intensity = clamp((_cosTheta - _light.m_outerCutOff) / _epsilon, 0.0f, 1.0f); 
	// 点方向光线到outerCutOff的夹角占模糊边缘总角度的比例（以上都是用余弦值近似计算角度，这两个成反比）

	return ((_diffuse + _specular) * _intensity + _ambient) * _attenuation;
}

void main(){
	vec3 _normal = normalize(outNormal);
	vec3 _viewDir = normalize(view_pos - outFragPos);

	// 计算平行光
	vec3 _result = calculateDir(_dirLight, _normal, _viewDir);

	// 计算点光源
	_result += calculatePoint(_pointLight, _normal, _viewDir, outFragPos);

	// 计算聚光灯
	_result += calculateSpot(_spotLight, _normal, _viewDir, outFragPos);

	FragColor = vec4(_result, 1.0f);
};