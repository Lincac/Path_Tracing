#include"Sphere.h"
#include"Objects.h"
#include"BVH_SAH.h"
#include"Solid_Texture.h"
#include"Lambertian.h"
#include"Light.h"
#include"Triangle.h"
#include"Camera.h"
#include<ctime>
#include<omp.h>

std::clock_t start,end;

const int width = 500;
const int height = 500;
const int samplePixel = 50;

const double near = .0001;
const double far = 100.0;

const int depth = 50;

vec3 bgcolor = vec3(0);

vec3 LTE(const Ray& ray, const Objects& world, int depth) {
	Recoord hit;

	if (depth < 0) return vec3(0);

	if (!world.hit(ray, near, far, hit)) return bgcolor;

	Ray scatter_ray;
	vec3 albedo = hit.material->get_albedo(hit.u,hit.v); // self's color
	vec3 emissived = hit.material->emitted(hit.u,hit.v); // 
	double pdf;

	if (!hit.material->scatter(scatter_ray,hit,pdf))
		return emissived;

	return emissived + albedo * hit.material->scatter_pdf(scatter_ray,hit) *  LTE(scatter_ray, world, depth - 1) / pdf;
}

//Objects Get_World() {
//	Objects World;
//
//	std::shared_ptr<Material> lambertian = std::make_shared<Lambertian>();
//	std::shared_ptr<Material> metallic = std::make_shared<Metallic>(.3);
//	std::shared_ptr<Material> dielectric = std::make_shared< Dielectric>(1.5);
//
//	std::shared_ptr<Object> light = std::make_shared<Sphere>(vec3(20, 20, 1), vec3(1) * 20, 5, lambertian, vec3(1));
//	World.add(light);
//
//	std::shared_ptr<Object> sphere = std::make_shared<Sphere>(vec3(0, -1001, 0), vec3(.5, .5, .5), 1000, lambertian);
//	std::shared_ptr<Object> triangle = std::make_shared<Triangle>(vec3(-2, -.5, 1), vec3(2, -.5, 1), vec3(0, .5, -1), vec3(.1, .7, .7), lambertian);
//	World.add(sphere);
//	World.add(triangle);
//
//	return Objects(std::make_shared<BVH_SAH>(World,5));
//}

Objects Cornell_Box() {
	Objects World;

	std::shared_ptr<Texture> grey = std::make_shared<Solid_Texture>(vec3(.5, .5, .5));
	std::shared_ptr<Texture> lightblue = std::make_shared<Solid_Texture>(vec3(.1, .7, .7));
	std::shared_ptr<Texture> red = std::make_shared<Solid_Texture>(vec3(1, 0, 0));
	std::shared_ptr<Texture> green = std::make_shared<Solid_Texture>(vec3(0, 1, 0));
	std::shared_ptr<Texture> white = std::make_shared<Solid_Texture>(vec3(1));

	//obj
	std::shared_ptr<Material> lambertian = std::make_shared<Lambertian>(lightblue);
	World.add(std::make_shared<Sphere>(vec3(0, -.5, 0), 0.5, lambertian));

	//bottom
	std::shared_ptr<Material> lambertianbottom = std::make_shared<Lambertian>(grey);
	World.add(std::make_shared<Triangle>(vec3(1, -1, 1), vec3(-1, -1, -1), vec3(-1, -1, 1), lambertianbottom));
	World.add(std::make_shared<Triangle>(vec3(1, -1, 1), vec3(1, -1, -1), vec3(-1, -1, -1), lambertianbottom));
	// top
	std::shared_ptr<Material> lambertiantop = std::make_shared<Lambertian>(grey);
	World.add(std::make_shared<Triangle>(vec3(1, 1, 1), vec3(-1, 1, 1), vec3(-1, 1, -1), lambertiantop));
	World.add(std::make_shared<Triangle>(vec3(1, 1, 1), vec3(-1, 1, -1), vec3(1, 1, -1), lambertiantop));
	// back
	std::shared_ptr<Material> lambertianback = std::make_shared<Lambertian>(grey);
	World.add(std::make_shared<Triangle>(vec3(1, -1, -1), vec3(-1, 1, -1), vec3(-1, -1, -1), lambertianback));
	World.add(std::make_shared<Triangle>(vec3(1, -1, -1), vec3(1, 1, -1), vec3(-1, 1, -1), lambertianback));
	// left
	std::shared_ptr<Material> lambertianleft = std::make_shared<Lambertian>(red);
	World.add(std::make_shared<Triangle>(vec3(-1, -1, -1), vec3(-1, 1, 1), vec3(-1, -1, 1),lambertianleft));
	World.add(std::make_shared<Triangle>(vec3(-1, -1, -1), vec3(-1, 1, -1), vec3(-1, 1, 1), lambertianleft));
	// right
	std::shared_ptr<Material> lambertianright = std::make_shared<Lambertian>(green);
	World.add(std::make_shared<Triangle>(vec3(1, 1, 1), vec3(1, -1, -1), vec3(1, -1, 1), lambertianright));
	World.add(std::make_shared<Triangle>(vec3(1, -1, -1), vec3(1, 1, 1), vec3(1, 1, -1), lambertianright));

	// light
	std::shared_ptr<Material> light = std::make_shared<Light>(white);
	World.add(std::make_shared<Triangle>(vec3(0.4, 0.99, 0.4), vec3(-0.4, 0.99, -0.4), vec3(-0.4, 0.99, 0.4),light,vec3(1)));
	World.add(std::make_shared<Triangle>(vec3(0.4, 0.99, 0.4), vec3(0.4, 0.99, -0.4), vec3(-0.4, 0.99, -0.4),light,vec3(1)));

	std::cout << "Total Shape : " << World.objs.size() << std::endl;
	std::cout << "Sample times : " << samplePixel << ",  Bound times : " << depth << std::endl;

	return Objects(std::make_shared<BVH_SAH>(World,5));
}

int main() {

	double* color = new double[width * height * 3];
	memset(color, 0.0, sizeof(double) * width * height * 3);

	Objects world = Cornell_Box();

	start = clock();

	//omp_set_num_threads(50); // 线程个数
	//#pragma omp parallel for

	for (int k = 0; k < samplePixel; k++)
	{
		double* image = color;
		for (int i = height - 1; i >= 0; i--)
		{
			for (int j = 0; j < width; j++)
			{
				auto u = (double(j + random01(engine)) / (width - 1.)) - .5;
				auto v = (double(i + random01(engine)) / (height - 1.)) - .5; // 先将int类型转为double，否则int相除向下取整
				u *= double(width) / height;

				Ray ray;
				ray.origin = vec3(0, 0, 3.1);
				ray.direction = unit_vector(vec3(u, v, -1));

				vec3 color = LTE(ray, world,depth);

				*image += color[0]; image++;
				*image += color[1]; image++;
				*image += color[2]; image++;
			}
		}
	}

	sotrage_image(color, width, height, samplePixel);

	end = clock();
	std::cout << "Done! Time :" << (double)(end - start) / CLOCKS_PER_SEC << std::endl;

	system("start RayTracing.ppm");

	delete[width * height * 3] color;
}