#pragma once

#ifndef GLOBAL_H
#define GLOBAL_H

#include"vec3.h"

#include<iostream>
#include<time.h>
#include<random>

std::uniform_real_distribution<double> random01(0.0, 1.0);
std::default_random_engine engine(time(0));

inline double clamp(double x,double min, double max) {
	if (x < min) return min;
	if (x > max) return max;
	return x;
}

inline double ffmin(double a, double b) { return a > b ? b : a; }
inline double ffmax(double a, double b) { return a < b ? b : a; }

void sotrage_image(double* color,int width,int height,int sample) {
	FILE* file = fopen("RayTracing.ppm", "wb");
	(void)fprintf(file, "P6\n%d %d\n255\n", width, height);

	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++) {

			static unsigned char RGB[3]{};
			RGB[0] = static_cast<unsigned char>(clamp(pow(*color / sample, 1 / 2.2) * 255, 0, 255)); color++;
			RGB[1] = static_cast<unsigned char>(clamp(pow(*color / sample, 1 / 2.2) * 255, 0, 255)); color++;
			RGB[2] = static_cast<unsigned char>(clamp(pow(*color / sample, 1 / 2.2) * 255, 0, 255)); color++;

			fwrite(RGB, 1, 3, file);
		}
	}
	fclose(file);
}

#endif // !GLOBAL_H
