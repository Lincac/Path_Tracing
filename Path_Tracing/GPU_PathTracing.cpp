#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include"TextureGPU.h"-
#include"Shader.h"
#include <algorithm>

#define STB_IMAGE_IMPLEMENTATION
#include<stb_image.h>

#define INF 114514.0
#define FFF 11451419.19

unsigned int WIDTH = 700;
unsigned int HEIGHT = 700;

const glm::vec3 camera = glm::vec3(0, 0, 3);

int frameCounter = 0;

struct Material
{
	glm::vec3 emissive = glm::vec3(0, 0, 0);  // 作为光源时的发光颜色
	glm::vec3 baseColor = glm::vec3(1, 1, 1);
	float subsurface = 0.0;
	float metallic = 0.0;
	float specular = 0.0;
	float specularTint = 0.0;
	float roughness = 0.0;
	float anisotropic = 0.0;
	float sheen = 0.0;
	float sheenTint = 0.0;
	float clearcoat = 0.0;
	float clearcoatGloss = 0.0;
	float IOR = 1.0;
	float transmission = 0.0;
};

struct Triangle
{
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;

	Material material;
};

struct Triangle_Code
{
	glm::vec3 p1, p2, p3;
	glm::vec3 n1, n2, n3;
	glm::vec3 emissive;
	glm::vec3 baseColor;
	glm::vec3 param1;
	glm::vec3 param2;
	glm::vec3 param3;
	glm::vec3 param4;
};

struct BVHNode
{
	int left, right;
	int n, startindex;
	glm::vec3 AA, BB;
};

struct BVHNode_Code
{
	glm::vec3 childs, leafInfo;
	glm::vec3 AA, BB;
};

void RenderQuad();
void processInput(GLFWwindow* window);
float* calculateHdrCache(const char* path, int& w, int& h);
void framebuffercallback(GLFWwindow* window, int width, int height);
void LoadModel(const char* path, std::vector<Triangle>& triangles, glm::mat4 trans, Material material);
int BuildBVH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int start, int end, int num);
int main() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "GPU_PathTracing", NULL, NULL);
	if (window == NULL)
	{
		std::cerr << "Create window error\n";
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffercallback);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cerr << "load glad error\n";
		glfwTerminate();
		return -1;
	}

	std::vector<Triangle> triangles;

	glm::mat4 model = glm::mat4(1);
	model = glm::scale(model, glm::vec3(1));
	model = glm::translate(model, glm::vec3(0,-1.5,0));
	Material material;
	material.baseColor = glm::vec3(1, 0.73, 0.25);
	material.roughness = 0.5;
	material.specular = 1;
	material.metallic = 1;
	material.clearcoat = 0;
	LoadModel("resource/Model/bunny.obj", triangles, model, material);

	model = glm::mat4(1);
	model = glm::scale(model, glm::vec3(5));
	model = glm::translate(model, glm::vec3(0));
	material.baseColor = glm::vec3(1);
	material.roughness = 1;
	material.specular = 0;
	material.metallic = 0;
	material.clearcoat = 0;
	LoadModel("resource/Model/quad.obj", triangles, model, material);

	int nTriangle = triangles.size();
	std::cout << "模型读取完成 ,共有 : " << nTriangle << " 个三角形" << std::endl;

	// BVH树建立
	static std::vector<BVHNode> nodes;
	BuildBVH(triangles, nodes, 0, triangles.size() - 1, 16);
	int nNode = nodes.size();
	std::cout << "BVH建立完成 ,共有 : " << nNode << " 个节点" << std::endl;

	// 三角形数据编码
	std::vector<Triangle_Code> triangle_code(nTriangle);
	for (int i = 0; i < nTriangle; i++)
	{
		Triangle& triangle = triangles[i];
		Material& material = triangles[i].material;

		triangle_code[i].p1 = triangle.p1;
		triangle_code[i].p2 = triangle.p2;
		triangle_code[i].p3 = triangle.p3;

		triangle_code[i].n1 = triangle.n1;
		triangle_code[i].n2 = triangle.n2;
		triangle_code[i].n3 = triangle.n3;

		triangle_code[i].emissive = material.emissive;
		triangle_code[i].baseColor = material.baseColor;
		triangle_code[i].param1 = glm::vec3(material.subsurface, material.metallic, material.specular);
		triangle_code[i].param2 = glm::vec3(material.specularTint, material.roughness, material.anisotropic);
		triangle_code[i].param3 = glm::vec3(material.sheen, material.sheenTint, material.clearcoat);
		triangle_code[i].param4 = glm::vec3(material.clearcoatGloss, material.IOR, material.transmission);
	}

	//// 存储三角形数据
	GLuint TB0;
	glGenBuffers(1, &TB0);
	glBindBuffer(GL_TEXTURE_BUFFER, TB0);
	glBufferData(GL_TEXTURE_BUFFER, triangle_code.size() * sizeof(Triangle_Code), &triangle_code[0], GL_STATIC_DRAW);
	GLuint TriangleBuffer;
	glGenTextures(1, &TriangleBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, TriangleBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TB0);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	// BVH数据编码
	std::vector<BVHNode_Code> bvhnode_code(nNode);
	for (int i = 0; i < nNode; i++)
	{
		bvhnode_code[i].childs = glm::vec3(nodes[i].left, nodes[i].right, 0);
		bvhnode_code[i].leafInfo = glm::vec3(nodes[i].n, nodes[i].startindex, 0);
		bvhnode_code[i].AA = nodes[i].AA;
		bvhnode_code[i].BB = nodes[i].BB;
	}

	// 存储BVH数据
	GLuint TB1;
	glGenBuffers(1, &TB1);
	glBindBuffer(GL_TEXTURE_BUFFER, TB1);
	glBufferData(GL_TEXTURE_BUFFER, bvhnode_code.size() * sizeof(BVHNode_Code), &bvhnode_code[0], GL_STATIC_DRAW);
	GLuint BVHNodeBuffer;
	glGenTextures(1, &BVHNodeBuffer);
	glBindTexture(GL_TEXTURE_BUFFER, BVHNodeBuffer);
	glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, TB1);
	glBindBuffer(GL_TEXTURE_BUFFER, 0);
	glBindTexture(GL_TEXTURE_BUFFER, 0);

	// HDR环境贴图
	GLuint HDRMap = loadHDRTexture("resource/texture/HDR/peppermint_powerplant_4k.hdr");

	// 预计算hdrPDF
	int hdrW, hdrH;
	float* Cache = calculateHdrCache("resource/texture/HDR/peppermint_powerplant_4k.hdr", hdrW, hdrH);
	GLuint HDRCache = generateTexture2D(hdrW, hdrH);
	glBindTexture(GL_TEXTURE_2D, HDRCache);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, hdrW, hdrH, 0, GL_RGB, GL_FLOAT, Cache);
	glBindTexture(GL_TEXTURE_2D, 0);

	// 路径追踪
	Shader pathTracing("shader/GPU_PathTracing.vs", "shader/GPU_PathTracing.fs"); 
	pathTracing.use();
	pathTracing.setInt("LastColor", 0);
	pathTracing.setInt("hdrMap", 1);
	pathTracing.setInt("hdrCache", 2);
	pathTracing.setInt("Triangles", 3);
	pathTracing.setInt("BVHNodes", 4);
	pathTracing.setInt("width", WIDTH);
	pathTracing.setInt("height", HEIGHT);
	pathTracing.setInt("hdrResolution", hdrW);
	pathTracing.setInt("nTriangle", nTriangle);
	pathTracing.setInt("nBVHNode", nNode);
	GLuint TracingFBO;
	glGenFramebuffers(1, &TracingFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, TracingFBO);
	GLuint TracingColor = generateTexture2D(WIDTH, HEIGHT);
	glBindTexture(GL_TEXTURE_2D, TracingColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TracingColor, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 保存上一帧颜色
	GLuint LastFBO;
	glGenFramebuffers(1, &LastFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, LastFBO);
	GLuint LastColor = generateTexture2D(WIDTH, HEIGHT);
	glBindTexture(GL_TEXTURE_2D, LastColor);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LastColor, 0);
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// 最终输出
	Shader finalPass("shader/Pass.vs", "shader/Pass3.fs");
	finalPass.use();
	finalPass.setInt("scene", 0);

	while (!glfwWindowShouldClose(window))
	{
		processInput(window);

		glBindFramebuffer(GL_FRAMEBUFFER, TracingFBO);
		glClear(GL_COLOR_BUFFER_BIT);
		pathTracing.use();
		pathTracing.setVec3("camPos", camera);
		pathTracing.setInt("frameCounter", frameCounter++);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, LastColor);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, HDRMap);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, HDRCache);
		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_BUFFER, TriangleBuffer);
		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_BUFFER, BVHNodeBuffer);
		RenderQuad();

		glBindFramebuffer(GL_READ_FRAMEBUFFER, TracingFBO);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, LastFBO);
		glBlitFramebuffer(0, 0, WIDTH, HEIGHT, 0, 0, WIDTH, HEIGHT, GL_COLOR_BUFFER_BIT, GL_LINEAR);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClear(GL_COLOR_BUFFER_BIT);
		finalPass.use();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, TracingColor);
		RenderQuad();

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
};

void framebuffercallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
};

unsigned int quadVAO = 0;
void RenderQuad() {
	if (quadVAO == 0)
	{
		unsigned int quadVBO;
		float quadVertices[] = {
			// positions        // texture Coords
			-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
			-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
			 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
			 1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
		};
		glGenVertexArrays(1, &quadVAO);
		glBindVertexArray(quadVAO);

		glGenBuffers(1, &quadVBO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	}

	glBindVertexArray(quadVAO);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glBindVertexArray(0);
};

// 计算 HDR 贴图相关缓存信息
float* calculateHdrCache(const char* path, int& w, int& h) {

	int width, height, nrComponents;
	float* HDR = stbi_loadf(path, &width, &height, &nrComponents, 0);

	w = width;
	h = height;

	float lumSum = 0.0;

	// 初始化 h 行 w 列的概率密度 pdf 并 统计总亮度
	std::vector<std::vector<float>> pdf(height);
	for (auto& line : pdf) line.resize(width);
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			float R = HDR[3 * (i * width + j)];
			float G = HDR[3 * (i * width + j) + 1];
			float B = HDR[3 * (i * width + j) + 2];
			float lum = 0.2 * R + 0.7 * G + 0.1 * B;
			pdf[i][j] = lum;
			lumSum += lum;
		}
	}

	stbi_image_free(HDR);

	// 概率密度归一化
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			pdf[i][j] /= lumSum;

	// 累加每一列得到 x 的边缘概率密度
	std::vector<float> pdf_x_margin;
	pdf_x_margin.resize(width);
	for (int j = 0; j < width; j++)
		for (int i = 0; i < height; i++)
			pdf_x_margin[j] += pdf[i][j];

	// 计算 x 的边缘分布函数
	std::vector<float> cdf_x_margin = pdf_x_margin;
	for (int i = 1; i < width; i++)
		cdf_x_margin[i] += cdf_x_margin[i - 1];

	// 计算 y 在 X=x 下的条件概率密度函数
	std::vector<std::vector<float>> pdf_y_condiciton = pdf;
	for (int j = 0; j < width; j++)
		for (int i = 0; i < height; i++)
			pdf_y_condiciton[i][j] /= pdf_x_margin[j];

	// 计算 y 在 X=x 下的条件概率分布函数
	std::vector<std::vector<float>> cdf_y_condiciton = pdf_y_condiciton;
	for (int j = 0; j < width; j++)
		for (int i = 1; i < height; i++)
			cdf_y_condiciton[i][j] += cdf_y_condiciton[i - 1][j];

	// cdf_y_condiciton 转置为按列存储
	// cdf_y_condiciton[i] 表示 y 在 X=i 下的条件概率分布函数
	std::vector<std::vector<float>> temp = cdf_y_condiciton;
	cdf_y_condiciton = std::vector<std::vector<float>>(width);
	for (auto& line : cdf_y_condiciton) line.resize(height);
	for (int j = 0; j < width; j++)
		for (int i = 0; i < height; i++)
			cdf_y_condiciton[j][i] = temp[i][j];

	// 穷举 xi_1, xi_2 预计算样本 xy
	// sample_x[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 x
	// sample_y[i][j] 表示 xi_1=i/height, xi_2=j/width 时 (x,y) 中的 y
	// sample_p[i][j] 表示取 (i, j) 点时的概率密度
	std::vector<std::vector<float>> sample_x(height);
	for (auto& line : sample_x) line.resize(width);
	std::vector<std::vector<float>> sample_y(height);
	for (auto& line : sample_y) line.resize(width);
	std::vector<std::vector<float>> sample_p(height);
	for (auto& line : sample_p) line.resize(width);
	for (int j = 0; j < width; j++) {
		for (int i = 0; i < height; i++) {
			float xi_1 = float(i) / height;
			float xi_2 = float(j) / width;

			// 用 xi_1 在 cdf_x_margin 中 lower bound 得到样本 x
			int x = std::lower_bound(cdf_x_margin.begin(), cdf_x_margin.end(), xi_1) - cdf_x_margin.begin();
			// 用 xi_2 在 X=x 的情况下得到样本 y
			int y = std::lower_bound(cdf_y_condiciton[x].begin(), cdf_y_condiciton[x].end(), xi_2) - cdf_y_condiciton[x].begin();

			// 存储纹理坐标 xy 和 xy 位置对应的概率密度
			sample_x[i][j] = float(x) / width;
			sample_y[i][j] = float(y) / height;
			sample_p[i][j] = pdf[i][j];
		}
	}

	// 整合结果到纹理
	// R,G 通道存储样本 (x,y) 而 B 通道存储 pdf(i, j)
	float* cache = new float[width * height * 3];
	//for (int i = 0; i < width * height * 3; i++) cache[i] = 0.0;

	for (int i = 0; i < height; i++) {
		for (int j = 0; j < width; j++) {
			cache[3 * (i * width + j)] = sample_x[i][j];        // R
			cache[3 * (i * width + j) + 1] = sample_y[i][j];    // G
			cache[3 * (i * width + j) + 2] = sample_p[i][j];    // B
		}
	}

	return cache;
}

int BuildBVH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int start, int end, int num) {
	//为什么常引用出错？
	// 将三角形排序后进行节点连接的，常引用导致原来三角形序列并还是原来的，而节点中的三角形却是排序后的
	if (start > end) return 0;

	//auto templist = triangles;

	nodes.push_back(BVHNode()); // 结构体也是特殊的类
	int id = nodes.size() - 1;
	nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].startindex = 0;
	nodes[id].AA = glm::vec3(FFF);
	nodes[id].BB = glm::vec3(-FFF);

	for (int i = start; i <= end; i++)
	{
		glm::vec3 minvec = glm::min(triangles[i].p1, glm::min(triangles[i].p2, triangles[i].p3));
		nodes[id].AA = glm::min(minvec, nodes[id].AA);

		glm::vec3 maxvec = glm::max(triangles[i].p1, glm::max(triangles[i].p2, triangles[i].p3));
		nodes[id].BB = glm::max(maxvec, nodes[id].BB);
	}

	if ((end - start + 1) <= num) {
		nodes[id].n = (end - start + 1);
		nodes[id].startindex = start;

		return id;
	}

	double Cost = INF;
	int Axis = 0;
	int Split = (start + end) / 2;
	for (int axis = 0; axis < 3; axis++)
	{
		if (axis == 0) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
			glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
			glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

			return a_aa.x < b_aa.x; // up sort
			});
		if (axis == 1) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
			glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
			glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

			return a_aa.y < b_aa.y; // up sort
			});
		if (axis == 2) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
			glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
			glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

			return a_aa.z < b_aa.z; // up sort
			});

		//  left Prefixes and  
		std::vector<glm::vec3> leftmin(end - start + 1, glm::vec3(INF));
		std::vector<glm::vec3> leftmax(end - start + 1, glm::vec3(-INF));
		for (int i = start; i <= end; i++)
		{
			int bias = i == start ? 0 : 1;

			glm::vec3 tempaa = glm::min(triangles[i].p1, glm::min(triangles[i].p2, triangles[i].p3));
			glm::vec3 tempbb = glm::max(triangles[i].p1, glm::max(triangles[i].p2, triangles[i].p3));
			leftmin[i - start] = glm::min(leftmin[i - start - bias], tempaa);
			leftmax[i - start] = glm::max(leftmax[i - start - bias], tempbb);
		}

		//  right Prefixes and  
		std::vector<glm::vec3> rightmin(end - start + 1, glm::vec3(INF));
		std::vector <glm::vec3> rightmax(end - start + 1, glm::vec3(-INF));
		for (int i = end; i >= start; i--)
		{
			int bias = i == end ? 0 : 1;
			glm::vec3 tempaa = glm::min(triangles[i].p1, glm::min(triangles[i].p2, triangles[i].p3));
			glm::vec3 tempbb = glm::max(triangles[i].p1, glm::max(triangles[i].p2, triangles[i].p3));
			rightmin[i - start] = glm::min(rightmin[i - start + bias], tempaa);
			rightmax[i - start] = glm::max(rightmax[i - start + bias], tempbb);
		}

		double cost = INF;
		int split = start;
		for (int i = start; i <= end - 1; i++)
		{
			double x, y, z;

			glm::vec3 leftaa = leftmin[i - start];
			glm::vec3 leftbb = leftmax[i - start];
			x = leftbb.x - leftaa.x;
			y = leftbb.y - leftaa.y;
			z = leftbb.z - leftaa.z;
			double lefts = 2.0 * (x * y + x * z + y * z);
			double leftcost = lefts * (i - start + 1);

			glm::vec3 rightaa = rightmin[i - start + 1];
			glm::vec3 rightbb = rightmax[i - start + 1];
			x = rightbb.x - rightaa.x;
			y = rightbb.y - rightaa.y;
			z = rightbb.z - rightaa.z;
			double rights = 2.0 * (x * y + x * z + y * z);
			double rightcost = rights * (end - start);

			double totalcost = leftcost + rightcost;
			if (totalcost < cost) {
				cost = totalcost;
				split = i;
			}
		}

		if (cost < Cost) {
			Cost = cost;
			Split = split;
			Axis = axis;
		}
	}

	if (Axis == 0) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
		glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
		glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

		return a_aa.x < b_aa.x; // up sort
		});
	if (Axis == 1) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
		glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
		glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

		return a_aa.y < b_aa.y; // up sort
		});
	if (Axis == 2) std::sort(triangles.begin() + start, triangles.begin() + end + 1, [](const Triangle& a, const Triangle& b) {
		glm::vec3 a_aa = (a.p1 + a.p2 + a.p3) / glm::vec3(3);
		glm::vec3 b_aa = (b.p1 + b.p2 + b.p3) / glm::vec3(3);

		return a_aa.z < b_aa.z; // up sort
		});

	nodes[id].left = BuildBVH(triangles, nodes, start, Split, num);
	nodes[id].right = BuildBVH(triangles, nodes, Split + 1, end, num);

	return id;
};

void LoadModel(const char* path, std::vector<Triangle>& triangles, glm::mat4 trans, Material material) {
	std::vector<glm::vec3> vertices;
	std::vector<GLuint> indexs;

	std::ifstream file(path);
	std::string line;
	if (!file.is_open())
	{
		std::cerr << path << " can not open : \n";
		exit(-1);
	}

	glm::vec3 maxxyz = glm::vec3(-INF);
	glm::vec3 minxyz = glm::vec3(INF);
	while (std::getline(file, line))
	{
		std::istringstream str(line);
		std::string type;

		GLfloat x, y, z;
		int v0, v1, v2;

		str >> type;
		if (type == "v")
		{
			str >> x >> y >> z;
			maxxyz.x = x > maxxyz.x ? x : maxxyz.x;
			maxxyz.y = y > maxxyz.y ? y : maxxyz.y;
			maxxyz.z = z > maxxyz.z ? z : maxxyz.z;

			minxyz.x = x < minxyz.x ? x : minxyz.x;
			minxyz.y = y < minxyz.y ? y : minxyz.y;
			minxyz.z = z < minxyz.z ? z : minxyz.z;

			vertices.push_back(glm::vec3(x, y, z));
		}
		else if (type == "f") {
			str >> v0 >> v1 >> v2;

			indexs.push_back(v0 - 1);
			indexs.push_back(v1 - 1);
			indexs.push_back(v2 - 1);
		}
	}

	glm::vec3 len = maxxyz - minxyz;
	float maxaxis = fmax(len.x, fmax(len.y, len.z));
	for (int i = 0; i < indexs.size(); i += 3)
	{
		Triangle temp;
		temp.p1 = vertices[indexs[i + 0]] / maxaxis;  // 单位化
		temp.p2 = vertices[indexs[i + 1]] / maxaxis;
		temp.p3 = vertices[indexs[i + 2]] / maxaxis;

		temp.p1 = glm::vec3(trans * glm::vec4(temp.p1, 1));
		temp.p2 = glm::vec3(trans * glm::vec4(temp.p2, 1));
		temp.p3 = glm::vec3(trans * glm::vec4(temp.p3, 1));

		glm::vec3 n = glm::normalize(glm::cross(temp.p3 - temp.p1, temp.p2 - temp.p1)); // glm 左手规则？
		temp.n1 = n;
		temp.n2 = n;
		temp.n3 = n;

		temp.material = material;

		triangles.push_back(temp);
	}
}