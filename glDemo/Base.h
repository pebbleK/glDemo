#pragma once
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<string>
#include<fstream>
#include<sstream>
#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>
#include<math.h>
#include<vector>
#include<map>

typedef unsigned int uint;
typedef unsigned char byte;

struct ffRGBA {
	byte m_r;
	byte m_g;
	byte m_b;
	byte m_a;

	ffRGBA(byte _r = 255,
		byte _g = 255,
		byte _b = 255,
		byte _a = 255) {
		m_r = _r;
		m_g = _g;
		m_b = _b;
		m_a = _a;
	}
};

template<typename T>
struct tVec3 {
	T m_x;
	T m_y;
	T m_z;
	tVec3(T _x = 0, T _y = 0, T _z = 0) {
		m_x = _x;
		m_y = _y;
		m_z = _z;
	}
};

template<typename T>
struct tVec2 {
	T m_x;
	T m_y;
	tVec2(T _x = 0, T _y = 0) {
		m_x = _x;
		m_y = _y;
	}
};

#define SINGLE_INSTANCE(className)	private:\
										static className* m_Instance;\
										className(const className& gw) = delete; \
										className& operator=(const className& ins) = delete;\
									public:\
										~className()\
										{\
											this->SINGLE_OVER();\
											delete m_Instance;\
										}\
										static className* getInstance()\
										{\
											if (m_Instance == nullptr)\
											{\
												m_Instance = new className(); \
											}\
											return m_Instance;\
										}\

#define SINGLE_INSTANCE_SET(className)  className*  className::m_Instance = nullptr;