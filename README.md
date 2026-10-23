# 计算机图形学作业

四次作业的源码。构建缓存、OpenCV 二进制库、编译出的 exe 和另一位同学的代码没有放进仓库。

## 技术栈与关键词

- 技术栈：C++、Eigen、OpenCV、OpenGL、GLFW、GLAD、Visual Studio
- 关键词：计算机图形学、软件光栅化、旋转、透视投影、Bresenham、Z-Buffer、重心插值、欧拉操作、B-Rep、半边结构、光线跟踪、Whitted、Phong

## 目录

| 目录 | 内容 | 入口 |
| --- | --- | --- |
| [hw1-rotation-projection](hw1-rotation-projection/) | 绕 Z 轴旋转、视图变换、透视投影和线框绘制 | `202330450581+黄锦中+HW1/CGHW1.sln` |
| [hw2-rasterizer-zbuffer](hw2-rasterizer-zbuffer/) | 三角形填充、重心插值和 Z-Buffer | `HW2/HW.sln` |
| [hw3-euler-operations](hw3-euler-operations/) | 用五个欧拉算子构造带方孔的立方体 | 源码在该目录根下，无解决方案 |
| [hw4-ray-tracer](hw4-ray-tracer/) | 反射、折射和 Phong 着色的光线跟踪 | `HW4自己/HW.sln` |

每次作业的具体操作和实现见对应目录里的 README。

## 依赖

- 作业 1、2、4 需要 Eigen。作业 2、4 已带 `include/Eigen`；作业 1 需要自己配置。
- 作业 1、2、4 需要 OpenCV。作业 2、4 的工程写的是 OpenCV 4.12，库文件按工程中的 `..\lib`，运行库放到解决方案下的 `bin`。
- 作业 3 需要 GLFW 和 GLAD，仓库里没有这两份依赖，也没有 Visual Studio 工程。
- 作业 3、4 的源文件是 GBK 编码。
