#pragma once

#ifndef RENDERPASS_H
#define RENDERPASS_H

#include"Shader.h"
#include<vector>

class RenderPass {
public:
	GLuint FBO = 0;
	GLuint texture;
	Shader shader;
	int width,height;

	void BindData() {
		glGenFramebuffers(1, &FBO);
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glBindTexture(GL_TEXTURE_2D, texture);
    	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
	}

	void Draw() {
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);
	}
};

#endif // !RENDERPASS_H
