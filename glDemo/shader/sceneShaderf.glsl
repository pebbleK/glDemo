#version 330 core
out vec4 FragColor;

in vec2 outUV;
in vec3 outFragPos;
in vec3 outNormal;

struct Material {
	sampler2D m_diffuse;
	sampler2D m_specular;

	float m_shiness;
	int hasSpecularMap;
};

uniform Material myMaterial;

// Parallel Light Source Structure
struct DirLight {
	vec3 m_direction; // Lighte source diretion

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;
};

// Point light source structure
struct PointLight {
	vec3 m_pos;

	vec3 m_ambient;
	vec3 m_diffuse;
	vec3 m_specular;

	float m_c;
	float m_l;
	float m_q;
};

// Spotlight source structure
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

vec3 getSpecularColor(){
	if (myMaterial.hasSpecularMap == 1) {
		return vec3(texture(myMaterial.m_specular, outUV));
	}

	return vec3(0.2f);
}

vec3 calculateDir(DirLight _light, vec3 _normal, vec3 _viewDir){
	// Normalize light direction
	vec3 _lightDir = normalize(_light.m_direction); // ƽ�й��Դ���������뷽��

	// ambient light intensity
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));

	// diffuse light intensity
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));

	// specular light intensity
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * getSpecularColor();

	return (_ambient + _diffuse + _specular);
}

vec3 calculatePoint(PointLight _light, vec3 _normal, vec3 _viewDir, vec3 _fragPos){
	vec3 _lightDir = normalize(_fragPos - _light.m_pos);
	// ambient light intensity
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));
	// diffuse light intensity
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));
	// specular light intensity
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * getSpecularColor();
	
	// Distance decay
	float _dis = length(_light.m_pos - _fragPos);
	float _attenuation = 1.0f / (_light.m_c + _light.m_l * _dis + _light.m_q * (_dis * _dis));

	return (_ambient + _diffuse + _specular) * _attenuation;
}

vec3 calculateSpot(SpotLight _light, vec3 _normal, vec3 _viewDir, vec3 _fragPos){
	vec3 _lightDir = normalize(_fragPos - _light.m_pos);
	// ambient light intensity
	vec3 _ambient = _light.m_ambient * vec3(texture(myMaterial.m_diffuse, outUV));
	// diffuse light intensity
	float _diff  = max(dot(_normal, -_lightDir), 0.0f);
	vec3 _diffuse = _diff * _light.m_diffuse * vec3(texture(myMaterial.m_diffuse, outUV));
	// specular light intensity
	vec3 _reflectDir = reflect(_lightDir, _normal);
	float _spec = pow(max(dot(_viewDir, _reflectDir), 0.0f), myMaterial.m_shiness);
	vec3 _specular = _spec * _light.m_specular * getSpecularColor();
	
	// Distance decay
	float _dis = length(_light.m_pos - _fragPos);
	float _attenuation = 1.0f / (_light.m_c + _light.m_l * _dis + _light.m_q * (_dis * _dis));
	// spotlight attenuation
	vec3 _spotDir = normalize(_light.m_direction); // Spotlight pointing direction
	float _cosTheta = dot(_lightDir, _spotDir); // Cosine of the angle between the point direction ray and the spotlight direction
	float _epsilon = _light.m_cutOff - _light.m_outerCutOff; // Cosine of the angle of the fuzzy edge
	float _intensity = clamp((_cosTheta - _light.m_outerCutOff) / _epsilon, 0.0f, 1.0f); 
	// The ratio of the angle between the light direction and outerCutOff to the total angle of the soft edge (all angles are approximated using cosine values, and these two are inversely proportional)

	return ((_diffuse + _specular) * _intensity + _ambient) * _attenuation;
}

void main(){
	vec3 _normal = normalize(outNormal);
	vec3 _viewDir = normalize(view_pos - outFragPos);

	// caculate parallel light
	vec3 _result = calculateDir(_dirLight, _normal, _viewDir);

	// caculate point light
	_result += calculatePoint(_pointLight, _normal, _viewDir, outFragPos);

	// caculate spotlight
	_result += calculateSpot(_spotLight, _normal, _viewDir, outFragPos);

	FragColor = vec4(_result, 1.0f);
};
