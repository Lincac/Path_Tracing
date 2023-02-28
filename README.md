# Path_Tracing

CPU端和GPU端光线追踪

CPU端效果

![test1](https://user-images.githubusercontent.com/83110022/221790584-ba0c03ca-87f6-467c-b27d-81ad23a67403.png)

GPU端效果
金属

![~E0CR7D@8CK5T8X7E_G_P9Q](https://user-images.githubusercontent.com/83110022/221790979-76ef0e24-d4bf-4424-959d-9e93086ff284.png)

光滑塑料

![3{U9)U99`QU$ {W1(5}_6LV](https://user-images.githubusercontent.com/83110022/221791110-02b501ad-5379-4b79-ad41-6bc64b5ba697.png)

玉石（看起来挺像）

![0U9I7WSC0A$J @I%}8BMIA9](https://user-images.githubusercontent.com/83110022/221791231-829adc5f-88e8-4cf4-b34a-000ca4ff8eb0.png)


CPU：
带有SAH加速的BVH结构
蒙特卡洛

GPU：
迪士尼BRDF
蒙特卡洛
低差异序列
重要性采样（镜面反射，漫反射，清漆，HDR环境图）
多重重要性采样

引用
光线追踪渲染实战（五）：低差异序列与重要性采样，加速收敛！ https://blog.csdn.net/weixin_44176696/article/details/119988866?spm=1001.2014.3001.5502
Ray Tracing: The Next Week V3.0中文翻译 https://zhuanlan.zhihu.com/p/129745508

关于版本更新
由于时间问题和技术问题，项目中仅为简单的光追渲染，日后有时间会继续优化
1.迪士尼拓展BSDF
2.CUDA加速
3.加入计算着色器
....

