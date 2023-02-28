#version 430 core

out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D LastColor;
uniform sampler2D hdrMap;
uniform sampler2D hdrCache;

uniform samplerBuffer triangles;
uniform samplerBuffer bvhnodes;

uniform int width;
uniform int height;
uniform int hdrResolution;

uniform vec3 camPos;
uniform int frameCounter;

uniform int nTriangle;
uniform int nNode;

const int MAXBounds = 2;
#define INF 114514.0
#define PI 3.1415926

#define PARA_TRIANGLE 12
#define PARA_BVHNODE 4

struct Ray{
    vec3 origin;
    vec3 direction;
};

struct Material {
    vec3 emissive;          
    vec3 baseColor;
    float subsurface;
    float metallic;
    float specular;
    float specularTint;
    float roughness;
    float anisotropic;
    float sheen;
    float sheenTint;
    float clearcoat;
    float clearcoatGloss;
    float IOR;
    float transmission;
};

struct Triangle{
    vec3 p1,p2,p3;
    vec3 n1,n2,n3;
    Material material;
};

struct BVHNode{
    int left,right;
    int n,startindex;
    vec3 AA,BB;
};

struct Sphere{
    vec3 position;
    float radius;
    Material material;
};

struct HitReCoord{
    bool hit;

    float t;
    vec3 position;
    vec3 normal;
    bool front;

    Material material;
};

mat3 rotateX(float theta) {
    float c = cos(theta);
    float s = sin(theta);
    return mat3(
        vec3(1, 0, 0),
        vec3(0, c, -s),
        vec3(0, s, c)
    );
}

Triangle getTriangle(int i){
    int offset = i * PARA_TRIANGLE;

    Triangle tempT;
    tempT.p1 = texelFetch(triangles,offset + 0).xyz;
    tempT.p2 = texelFetch(triangles,offset + 1).xyz;
    tempT.p3 = texelFetch(triangles,offset + 2).xyz;

    tempT.n1 = texelFetch(triangles,offset + 3).xyz;
    tempT.n2 = texelFetch(triangles,offset + 4).xyz;
    tempT.n3 = texelFetch(triangles,offset + 5).xyz;

    Material tempM;

    tempM.emissive = texelFetch(triangles,offset + 6).xyz;
    tempM.baseColor = texelFetch(triangles,offset + 7).xyz;

    vec3 param1 = texelFetch(triangles,offset + 8).xyz;
    vec3 param2 = texelFetch(triangles,offset + 9).xyz;
    vec3 param3 = texelFetch(triangles,offset + 10).xyz;
    vec3 param4 = texelFetch(triangles,offset + 11).xyz;

    tempM.subsurface = param1.x;
    tempM.metallic = param1.y;
    tempM.specular = param1.z;
    tempM.specularTint = param2.x;
    tempM.roughness = param2.y;
    tempM.anisotropic = param2.z;
    tempM.sheen = param3.x;
    tempM.sheenTint = param3.y;
    tempM.clearcoat = param3.z;
    tempM.clearcoatGloss = param4.x;
    tempM.IOR = param4.y;
    tempM.transmission = param4.z;

    tempT.material = tempM;

    return tempT;
}

BVHNode getBVHNode(int i){
    int offset = i * PARA_BVHNODE;

    BVHNode temp;

    vec3 childs = texelFetch(bvhnodes,offset + 0).xyz; // 不能用ivec3()?
    vec3 leafinfo = texelFetch(bvhnodes,offset + 1).xyz;
    temp.AA = texelFetch(bvhnodes,offset + 2).xyz;
    temp.BB = texelFetch(bvhnodes,offset + 3).xyz;

    temp.left = int(childs.x);
    temp.right = int(childs.y);
    temp.n = int(leafinfo.x);
    temp.startindex = int(leafinfo.y);

    return temp;
}

float HitSphere(Ray ray,Sphere sphere){
    vec3 oc = ray.origin - sphere.position;
	
	float a = dot(ray.direction, ray.direction);
	float b = 2.0 * dot(oc, ray.direction);
	float c = dot(oc, oc) - sphere.radius * sphere.radius;

	float delta = b * b - 4. * a * c;

    if(delta > 0.){
        float temp = (-b - sqrt(delta)) / (2.0 * a);  // 最近的点
        return temp;
    }
    
    return -1;
}

bool HitTriangle(Ray ray,Triangle triangle,inout HitReCoord hitinfo){
    vec3 N = normalize(cross(triangle.p2 - triangle.p1,triangle.p3 - triangle.p1));
    vec3 tempN = N;

    hitinfo.front = true;
    if(dot(tempN,ray.direction) > 0.){
        tempN *= -1;
        hitinfo.front = false;
    }

    float t = (dot(tempN,triangle.p1) - dot(ray.origin,tempN)) / dot(ray.direction,tempN);
    if(t < 0.00005) return false;

    vec3 position = ray.origin + t * ray.direction;
    vec3 c1 = cross(triangle.p2 - triangle.p1,position - triangle.p1);
    vec3 c2 = cross(triangle.p3 - triangle.p2,position - triangle.p2);
    vec3 c3 = cross(triangle.p1 - triangle.p3,position - triangle.p3);
    if(dot(c1,N) < 0 || dot(c2,N) < 0 || dot(c3,N) < 0) return false;

    if(t > hitinfo.t) return false;

    hitinfo.hit = true;
    hitinfo.t = t;
    hitinfo.position = position;
    
    float alpha = (-(position.x-triangle.p2.x)*(triangle.p3.y-triangle.p2.y) + (position.y-triangle.p2.y)*(triangle.p3.x-triangle.p2.x)) / (-(triangle.p1.x-triangle.p2.x-0.00005)*(triangle.p3.y-triangle.p2.y+0.00005) + (triangle.p1.y-triangle.p2.y+0.00005)*(triangle.p3.x-triangle.p2.x+0.00005));
    float beta  = (-(position.x-triangle.p3.x)*(triangle.p1.y-triangle.p3.y) + (position.y-triangle.p3.y)*(triangle.p1.x-triangle.p3.x)) / (-(triangle.p2.x-triangle.p3.x-0.00005)*(triangle.p1.y-triangle.p3.y+0.00005) + (triangle.p2.y-triangle.p3.y+0.00005)*(triangle.p1.x-triangle.p3.x+0.00005));
    float gama  = 1.0 - alpha - beta;
    vec3 Nsmooth = alpha * triangle.n1 + beta * triangle.n2 + gama * triangle.n3;
    Nsmooth = normalize(Nsmooth);
    hitinfo.normal = hitinfo.front ? -Nsmooth : Nsmooth;

    hitinfo.material = triangle.material;

    return true;
}

float HitAABB(Ray ray,vec3 aa,vec3 bb){
    vec3 t0 = (aa - ray.origin) / ray.direction;
    vec3 t1 = (bb - ray.origin) / ray.direction;

    vec3 t_min = min(t0,t1);
    vec3 t_max = max(t0,t1);

    float tmin = max(t_min.x,max(t_min.y,t_min.z));
    float tmax = min(t_max.x,min(t_max.y,t_max.z));

    return (tmax >= tmin) ? (tmin > 0) ?  tmin : tmax : -1;
}

void HitTriangleLeaf(Ray ray,int starti,int endi,inout HitReCoord hitinfo){
    for(int i = starti; i < endi ;i++){
        Triangle triangle = getTriangle(i);
        HitTriangle(ray,triangle,hitinfo);
    }
}

void HitBVH(Ray ray,inout HitReCoord hitinfo){
    int stack[1024];
    int sp = 0;
    stack[sp++] = 0;

    while(sp > 0){
        int currentID = stack[--sp];

        BVHNode node = getBVHNode(currentID);

        if(node.n > 0.){
            int starti = node.startindex;
            int endi = node.startindex + node.n;
            HitTriangleLeaf(ray,starti,endi,hitinfo);
        }

        float d1 = -INF;
        float d2 = -INF;
        if(node.left > 0){
            BVHNode leftNode = getBVHNode(node.left);
            d1 = HitAABB(ray,leftNode.AA,leftNode.BB);
        }
        if(node.right > 0){
            BVHNode rightNode = getBVHNode(node.right);
            d2 = HitAABB(ray,rightNode.AA,rightNode.BB);
        }

        if(d1 >0 && d2 > 0){
            if(d1 < d2){
                stack[sp++] = node.right;
                stack[sp++] = node.left;
            }else{
                stack[sp++] = node.left;
                stack[sp++] = node.right;
            }
        }else if(d1 > 0){
            stack[sp++] = node.left;
        }else if(d2 > 0){
            stack[sp++] = node.right;
        }
    }
}

uint seed = uint(
    uint((TexCoords.x * 0.5 + 0.5) * width)  * uint(1973) + 
    uint((TexCoords.y * 0.5 + 0.5) * height) * uint(9277) + 
    uint(frameCounter) * uint(26699)) | uint(1);

uint wang_hash(inout uint seed) {
    seed = uint(seed ^ uint(61)) ^ uint(seed >> uint(16));
    seed *= uint(9);
    seed = seed ^ (seed >> 4);
    seed *= uint(0x27d4eb2d);
    seed = seed ^ (seed >> 15);
    return seed;
}
 
float rand() {
    return float(wang_hash(seed)) / 4294967296.0;
}

vec3 random_unit_vector() {
	float a = rand() * 2. * PI;
	float z = rand() * 2. - 1.;
	float r = sqrt(1 - z * z);
	return vec3(r * cos(a), r * sin(a), z);
}

vec2 toSphericalCoord(vec3 v) {
    vec2 uv = vec2(atan(v.z, v.x), asin(v.y));
    uv /= vec2(2.0 * PI, PI);
    uv += 0.5;
    uv.y = 1.0 - uv.y;
    return uv;
}

// 低差异序列
// 1 ~ 8 维的 sobol 生成矩阵
const uint V[8*32] = {
    2147483648, 1073741824, 536870912, 268435456, 134217728, 67108864, 33554432, 16777216, 8388608, 4194304, 2097152, 1048576, 524288, 262144, 131072, 65536, 32768, 16384, 8192, 4096, 2048, 1024, 512, 256, 128, 64, 32, 16, 8, 4, 2, 1,
    2147483648, 3221225472, 2684354560, 4026531840, 2281701376, 3422552064, 2852126720, 4278190080, 2155872256, 3233808384, 2694840320, 4042260480, 2290614272, 3435921408, 2863267840, 4294901760, 2147516416, 3221274624, 2684395520, 4026593280, 2281736192, 3422604288, 2852170240, 4278255360, 2155905152, 3233857728, 2694881440, 4042322160, 2290649224, 3435973836, 2863311530, 4294967295,
    2147483648, 3221225472, 1610612736, 2415919104, 3892314112, 1543503872, 2382364672, 3305111552, 1753219072, 2629828608, 3999268864, 1435500544, 2154299392, 3231449088, 1626210304, 2421489664, 3900735488, 1556135936, 2388680704, 3314585600, 1751705600, 2627492864, 4008611328, 1431684352, 2147543168, 3221249216, 1610649184, 2415969680, 3892340840, 1543543964, 2382425838, 3305133397,
    2147483648, 3221225472, 536870912, 1342177280, 4160749568, 1946157056, 2717908992, 2466250752, 3632267264, 624951296, 1507852288, 3872391168, 2013790208, 3020685312, 2181169152, 3271884800, 546275328, 1363623936, 4226424832, 1977167872, 2693105664, 2437829632, 3689389568, 635137280, 1484783744, 3846176960, 2044723232, 3067084880, 2148008184, 3222012020, 537002146, 1342505107,
    2147483648, 1073741824, 536870912, 2952790016, 4160749568, 3690987520, 2046820352, 2634022912, 1518338048, 801112064, 2707423232, 4038066176, 3666345984, 1875116032, 2170683392, 1085997056, 579305472, 3016343552, 4217741312, 3719483392, 2013407232, 2617981952, 1510979072, 755882752, 2726789248, 4090085440, 3680870432, 1840435376, 2147625208, 1074478300, 537900666, 2953698205,
    2147483648, 1073741824, 1610612736, 805306368, 2818572288, 335544320, 2113929216, 3472883712, 2290089984, 3829399552, 3059744768, 1127219200, 3089629184, 4199809024, 3567124480, 1891565568, 394297344, 3988799488, 920674304, 4193267712, 2950604800, 3977188352, 3250028032, 129093376, 2231568512, 2963678272, 4281226848, 432124720, 803643432, 1633613396, 2672665246, 3170194367,
    2147483648, 3221225472, 2684354560, 3489660928, 1476395008, 2483027968, 1040187392, 3808428032, 3196059648, 599785472, 505413632, 4077912064, 1182269440, 1736704000, 2017853440, 2221342720, 3329785856, 2810494976, 3628507136, 1416089600, 2658719744, 864310272, 3863387648, 3076993792, 553150080, 272922560, 4167467040, 1148698640, 1719673080, 2009075780, 2149644390, 3222291575,
    2147483648, 1073741824, 2684354560, 1342177280, 2281701376, 1946157056, 436207616, 2566914048, 2625634304, 3208642560, 2720006144, 2098200576, 111673344, 2354315264, 3464626176, 4027383808, 2886631424, 3770826752, 1691164672, 3357462528, 1993345024, 3752330240, 873073152, 2870150400, 1700563072, 87021376, 1097028000, 1222351248, 1560027592, 2977959924, 23268898, 437609937
};

// 格林码 
uint grayCode(uint i) {
	return i ^ (i>>1);
}

// 生成第 d 维度的第 i 个 sobol 数
float sobol(uint d, uint i) {
    uint result = 0;
    uint offset = d * 32;
    for(uint j = 0; i != 0; i >>= 1, j++) {
        if((i & 1) == 1) result ^= V[j+offset];
    }
    return float(result) * (1.0f/float(0xFFFFFFFFU));
}

// 生成第 i 帧的第 b 次反弹需要的二维随机向量
vec2 sobolVec2(uint i, uint b) {
    float u = sobol(b*2, grayCode(i));
    float v = sobol(b*2+1, grayCode(i));
    return vec2(u, v);
}

// 获取 HDR 环境颜色
vec3 hdrColor(vec3 v) {
    vec2 uv = toSphericalCoord(normalize(v));
    vec3 color = texture(hdrMap, uv).rgb;
    color = min(color, vec3(10));
    return color;
}

// 将向量 v 投影到 N 的法向半球
vec3 toNormalHemisphere(vec3 v, vec3 N) {
    vec3 helper = vec3(1, 0, 0);
    if(abs(N.x)>0.999) helper = vec3(0, 0, 1);
    vec3 tangent = normalize(cross(N, helper));
    vec3 bitangent = normalize(cross(N, tangent));
    return v.x * tangent + v.y * bitangent + v.z * N;
}

// 镜面 镜面反射采样
vec3 SampleGTR2(float xi_1, float xi_2, vec3 V, vec3 N, float alpha) {
    
    float phi_h = 2.0 * PI * xi_1;
    float sin_phi_h = sin(phi_h);
    float cos_phi_h = cos(phi_h);

    float cos_theta_h = sqrt((1.0-xi_2)/(1.0+(alpha*alpha-1.0)*xi_2));
    float sin_theta_h = sqrt(max(0.0, 1.0 - cos_theta_h * cos_theta_h));

    // 采样 "微平面" 的法向量 作为镜面反射的半角向量 h 
    vec3 H = vec3(sin_theta_h*cos_phi_h, sin_theta_h*sin_phi_h, cos_theta_h);
    H = toNormalHemisphere(H, N);   // 投影到真正的法向半球

    // 根据 "微法线" 计算反射光方向
    vec3 L = reflect(-V, H);

    return L;
}

// 采样预计算的 HDR cache  //  hdr采样
vec3 SampleHdr(float xi_1, float xi_2) {
    vec2 xy = texture2D(hdrCache, vec2(xi_1, xi_2)).rg; // x, y
    xy.y = 1.0 - xy.y; // flip y

    // 获取角度
    float phi = 2.0 * PI * (xy.x - 0.5);    // [-pi ~ pi]
    float theta = PI * (xy.y - 0.5);        // [-pi/2 ~ pi/2]   

    // 球坐标计算方向
    vec3 L = vec3(cos(theta)*cos(phi), sin(theta), cos(theta)*sin(phi));

    return L;
}

// 多重重要性采样 ： 漫反射主要采样光源，镜面反射主要采样BRDF因此需要混合两种比值
float misMixWeight(float a, float b) {
    float t = a * a;
    return t / (b*b + t);
}

float SchlickFresnel(float u) { // 菲涅尔效应：不同角度反射程度不同
    float m = clamp(1-u, 0, 1);
    float m2 = m*m;
    return m2*m2*m; // pow(m,5)
}

float GTR2(float NdotH, float a) { // 法线分布，决定有多少法线和当前法线位于同一方向
    float a2 = a*a;
    float t = 1 + (a2-1)*NdotH*NdotH;
    return a2 / (PI * t*t);
}

float smithG_GGX(float NdotV, float alphaG) { // 自遮挡
    float a = alphaG*alphaG;
    float b = NdotV*NdotV;
    return 1 / (NdotV + sqrt(a + b - a*b));
}

vec3 BRDF(vec3 V,vec3 N,vec3 L,Material material){
    float NdotL = dot(N,L);
    float NdotV = dot(N,V);
    if(NdotL < 0 || NdotV < 0) return vec3(0);

    vec3 H = normalize(L + V);
    float NdotH = dot(N,H);
    float LdotH = dot(L,H);

    // 颜色
    vec3 Cdlin = material.baseColor;
    float Cdlum = 0.3 * Cdlin.r + 0.6 * Cdlin.g + 0.1 * Cdlin.b;
    vec3 Ctint = (Cdlum > 0) ? (Cdlin / Cdlum) : vec3(1);

    // 漫反射
    float Fd90 = 0.5 + 2.0 * LdotH * LdotH * material.roughness;
    float FL = SchlickFresnel(NdotL);
    float FV = SchlickFresnel(NdotV);
    float FD = mix(1.0,Fd90,FL) * mix(1.0,Fd90,FV);

    vec3 diffuse = FD * Cdlin / PI;

    // 镜面反射
    vec3 Cspec = material.specular * mix(vec3(1),Ctint,material.specularTint);
    vec3 Cspec0 = mix(0.08 * Cspec,Cdlin,material.metallic);

    float alpha = max(0.001, material.roughness * material.roughness);
    float Ds = GTR2(NdotH,alpha);
    float FH = SchlickFresnel(LdotH);
    vec3 Fs = mix(Cspec0,vec3(1),FH);
    float Gs = smithG_GGX(NdotL,material.roughness) * smithG_GGX(NdotV,material.roughness);

    vec3 specular = Gs * Fs * Ds ;
    
    return diffuse * (1.0 - material.metallic) + specular;
}

// 输入光线方向 L 获取 HDR 在该位置的概率密度
// hdr 分辨率为 4096 x 2048 --> hdrResolution = 4096
float HDR_pdf(vec3 L) {
    vec2 uv = toSphericalCoord(normalize(L));   // 方向向量转 uv 纹理坐标

    float pdf = texture2D(hdrCache, uv).b;      // 采样概率密度
    float theta = PI * (0.5 - uv.y);            // theta 范围 [-pi/2 ~ pi/2]
    float sin_theta = max(sin(theta), 1e-10);

    // 球坐标和图片积分域的转换系数
    float p_convert = float(hdrResolution * hdrResolution / 2) / (2.0 * PI * PI * sin_theta);  
    
    return pdf * p_convert;
}

vec3 PathTracing(Ray ray,inout HitReCoord hitinfo){
    vec3 throught = vec3(1);
    vec3 LO = vec3(0);

    for(int i=0;i<MAXBounds;i++){
        vec2 uv = sobolVec2(frameCounter + 1,i);
        float xi_1 = uv.x;
        float xi_2 = uv.y;
        float xi_3 = rand();

        vec3 N = hitinfo.normal;
        vec3 V = -ray.direction;

        // 采样HDR
        Ray hdrRay;
        hdrRay.origin = hitinfo.position;
        hdrRay.direction = normalize(SampleHdr(rand(),rand()));
        if(dot(N,hdrRay.direction) > 0.0){
            HitReCoord hdrHit;
            hdrHit.hit = false;
            hdrHit.t = INF;
            HitBVH(hdrRay,hdrHit);

            if(!hdrHit.hit){
                vec3 L = hdrRay.direction;

                // 采样光源
                vec3 hdr = hdrColor(L);
                float hdr_pdf = HDR_pdf(L);

                // 采样brdf
                vec3 brdf = BRDF(V,N,L,hitinfo.material);
                float brdf_pdf = dot(N,L) / PI;

                // 多重重要性采样 (对光源的采样在小光源效果很好，但是对大面积光源效果差，因此在对光源采样时结合BRDF采样权衡这两个值)
                float mis_weight = misMixWeight(hdr_pdf,brdf_pdf);
                
                LO += mis_weight * hdr * throught * brdf * dot(N,L) / hdr_pdf;
            }
        }

        float alpha_GTR2 = max(0.001, hitinfo.material.roughness * hitinfo.material.roughness);
        vec3 L = SampleGTR2(xi_1, xi_2, V, N, alpha_GTR2); // 出射光线

        float cosine_i = dot(N,L);
        if(cosine_i <= 0.0)  break;

        vec3 H = normalize(L + V);
        float alpha = max(0.001, hitinfo.material.roughness * hitinfo.material.roughness);
        float Ds = GTR2(dot(N,H), alpha); 
        float brdf_pdf = Ds * dot(N,H) / (4.0 * dot(L, H));
        if(brdf_pdf <= 0.0) break;

        vec3 brdf = BRDF(V,N,L,hitinfo.material);

        throught *= brdf * cosine_i / brdf_pdf;

        vec3 Le = hitinfo.material.emissive;
        LO += Le * throught;

        ray.origin = hitinfo.position;
        ray.direction = normalize(L);
        hitinfo.hit = false;
        hitinfo.t = INF;
        HitBVH(ray,hitinfo);

        if(!hitinfo.hit){
            // 采样光源
            vec3 hdr = hdrColor(ray.direction);
            float hdr_pdf = HDR_pdf(L);

            // 多重重要性采样
            float mis_weight = misMixWeight(brdf_pdf,hdr_pdf);

            LO += mis_weight * hdr * throught;
            return LO;
        }
    }

    return LO;
}


void main(){
    vec2 uv = TexCoords;
    uv -= vec2(0.5);

    vec2 offset = vec2((rand() - 0.5)/width,(rand() - 0.5)/height);

    Ray ray;
    ray.origin = camPos;
    ray.direction = normalize(vec3(uv + offset,camPos.z-1) - camPos) * rotateX(-0.3);

    HitReCoord hitinfo;
    hitinfo.hit = false;
    hitinfo.t = INF;
    HitBVH(ray,hitinfo);
    if(hitinfo.hit){
        vec3 Le = hitinfo.material.emissive;

        vec3 result = PathTracing(ray,hitinfo);
        result += Le;

        vec3 lastcolor = texture(LastColor,TexCoords).rgb;
        result = mix(lastcolor,result,1 / float(frameCounter + 1));

        FragColor = vec4(result,1);
    }else{
        FragColor = vec4(hdrColor(ray.direction),1);
    }
}