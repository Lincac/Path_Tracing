#pragma once

#ifndef TEXTUREGPU_H
#define TEXTUREGPU_H

#include<glad/glad.h>

#include<stb_image.h>

#include<iostream>
#include<string>
#include<vector>

#define INT_CEIL(n,d) (int)ceil((float)n/d)

 GLuint loadTexture(const char* path) {
	 GLuint texture;
	 glGenTextures(1, &texture);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, format == GL_RGBA ? GL_CLAMP_TO_EDGE : GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "error to load : " << path << std::endl;
		stbi_image_free(data);
	}
	return texture;
}

GLuint loadHDRTexture(const char* path) {
	GLuint texture;
	glGenTextures(1, &texture);

	//stbi_set_flip_vertically_on_load(true);
	int width, height, nrComponents;
	float* data = stbi_loadf(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "error to load hdrtexture" << std::endl;
	}

	return texture;
}

GLuint loadCubeMap(std::vector<std::string> path) {
	GLuint texture;
	glGenTextures(1, &texture);

	glBindTexture(GL_TEXTURE_CUBE_MAP,texture);
	int width, height, nrChannels;
	unsigned char* data = nullptr;
	for (int i = 0; i < path.size(); i++)
	{
		data = stbi_load(path[i].c_str(), &width, &height, &nrChannels, 0);  // c_str() 返回一个指向正规C字符串的 指针
		if (data) {
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			// 目标，多极渐远纹理（远处的物体不需要高分辨率的纹理），纹理存储形式
			// 源图的格式和数据类型，图像数据
			stbi_image_free(data); // 记得释放资源
		}
		else
		{
			std::cout << "error to load " + path[i] << std::endl;
			stbi_image_free(data);
		}
	}
	// 纹理参数设置
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //  当纹理被放大或者缩小时，采用的采样方式，线性过滤
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  //  纹理焕然的方式永远只采样边界值（0，1）
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);  // cube纹理有可能采样到两个面的连接处
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);// 围绕着xyz轴进行采样

	return texture;
}

GLuint generateTexture2D(int w, int h) {
	GLuint tex;
	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	return tex;
}



#endif // !TEXTUREGPU_H
