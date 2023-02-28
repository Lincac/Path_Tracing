#pragma once

#ifndef IMAGE_TEXTURE_H
#define IMAGE_TEXTURE_H

#include"Texture.h"

class Image_Texture : public Texture {
public:
	Image_Texture() {};
	Image_Texture(const char* path) {

	}
	
	virtual vec3 value(double u, double v) const override {
		return vec3(1);
	}
private:

};
#endif // !IMAGE_TEXTURE_H

