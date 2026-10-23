//
// Created by goksu on 4/6/19.
//
#include <algorithm>
#include "rasterizer.hpp"
#include <opencv2/opencv.hpp>
#include <math.h>
#include <stdexcept>
#include <Eigen/Eigen>

// 加载顶点缓冲区：存储顶点数据并返回唯一ID
rst::pos_buf_id rst::rasterizer::load_positions(const std::vector<Eigen::Vector3f>& positions)
{
    auto id = get_next_id();
    pos_buf.emplace(id, positions);
    return { id };
}

// 加载索引缓冲区：存储索引数据并返回唯一ID
rst::ind_buf_id rst::rasterizer::load_indices(const std::vector<Eigen::Vector3i>& indices)
{
    auto id = get_next_id();
    ind_buf.emplace(id, indices);
    return { id };
}

// Bresenham直线算法：高效光栅化直线（白色线条，确保线条连续无锯齿）
void rst::rasterizer::draw_line(Eigen::Vector3f begin, Eigen::Vector3f end)
{
    auto x1 = begin.x();
    auto y1 = begin.y();
    auto x2 = end.x();
    auto y2 = end.y();
    Eigen::Vector3f line_color = { 255, 255, 255 }; // 直线颜色：白色（RGB最大值）

    // 初始化直线绘制参数：坐标差、绝对值、误差项、终点坐标
    int x, y, dx, dy, dx1, dy1, px, py, xe, ye;
    dx = x2 - x1;
    dy = y2 - y1;
    dx1 = fabs(dx);
    dy1 = fabs(dy);
    px = 2 * dy1 - dx1;
    py = 2 * dx1 - dy1;

    // 处理x为主方向（水平方向更长）
    if (dy1 <= dx1) {
        // 确定绘制方向：从左到右（确保x递增）
        if (dx >= 0) { x = x1; y = y1; xe = x2; }
        else { x = x2; y = y2; xe = x1; }

        set_pixel(Eigen::Vector3f(x, y, 1.0f), line_color); // 绘制起点像素

        // 迭代绘制剩余像素，根据误差项调整y值
        for (; x < xe; x++) {
            if (px < 0) px += 2 * dy1; // 误差不足，仅沿x移动
            else {
                // 根据斜率方向调整y（同号递增，异号递减）
                y += (dx < 0 && dy < 0) || (dx > 0 && dy > 0) ? 1 : -1;
                px += 2 * (dy1 - dx1); // 误差重置
            }
            set_pixel(Eigen::Vector3f(x, y, 1.0f), line_color);
        }
    }
    // 处理y为主方向（垂直方向更长）
    else {
        // 确定绘制方向：从下到上（确保y递增）
        if (dy >= 0) { x = x1; y = y1; ye = y2; }
        else { x = x2; y = y2; ye = y1; }

        set_pixel(Eigen::Vector3f(x, y, 1.0f), line_color); // 绘制起点像素

        // 迭代绘制剩余像素，根据误差项调整x值
        for (; y < ye; y++) {
            if (py <= 0) py += 2 * dx1; // 误差不足，仅沿y移动
            else {
                // 根据斜率方向调整x（同号递增，异号递减）
                x += (dx < 0 && dy < 0) || (dx > 0 && dy > 0) ? 1 : -1;
                py += 2 * (dx1 - dy1); // 误差重置
            }
            set_pixel(Eigen::Vector3f(x, y, 1.0f), line_color);
        }
    }
}

// 辅助函数：3D顶点转4D齐次坐标（补充默认参数w=1.0f，解决“不接受1个参数”错误）
auto to_vec4(const Eigen::Vector3f& v3, float w)
{
    return Eigen::Vector4f(v3.x(), v3.y(), v3.z(), w);
}

// 绘制核心函数：MVP变换→透视除法→视口变换→线框光栅化（修复矩阵乘法类型不匹配）
void rst::rasterizer::draw(rst::pos_buf_id pos_buffer, rst::ind_buf_id ind_buffer, rst::Primitive type)
{
    // 仅支持三角形绘制，其他图元抛出异常
    if (type != rst::Primitive::Triangle)
    {
        throw std::runtime_error("暂不支持三角形以外的图元绘制！");
    }

    // 获取顶点缓冲区和索引缓冲区数据
    auto& buf = pos_buf[pos_buffer.pos_id];
    auto& ind = ind_buf[ind_buffer.ind_id];

    // 深度映射参数：将NDC的z值（[-1,1]）映射到[0,1]（深度缓冲用），与投影矩阵近远裁剪面（0.1,50）匹配
    float f1 = (50.0f - 0.1f) / 2.0f;
    float f2 = (50.0f + 0.1f) / 2.0f;

    // MVP矩阵：投影×视图×模型（矩阵乘法顺序不可改，右乘顶点时对应“模型→视图→投影”变换）
    Eigen::Matrix4f mvp = projection * view * model;

    // 遍历所有三角形索引，逐三角形处理（支持多个三角形批量绘制）
    for (auto& i : ind)
    {
        Triangle t;
        Eigen::Vector4f v[3]; // 存储3个顶点的4D齐次坐标（变换后）

        // 1. 对每个顶点应用MVP变换：先转4D齐次坐标（显式传w=1.0f），再做矩阵乘法（解决“*运算符不匹配”错误）
        // 修复点：to_vec4传入2个参数（v3和w=1.0f），确保函数参数匹配；mvp（4x4）右乘Vector4f（4x1），类型匹配
        v[0] = mvp * to_vec4(buf[i[0]], 1.0f);
        v[1] = mvp * to_vec4(buf[i[1]], 1.0f);
        v[2] = mvp * to_vec4(buf[i[2]], 1.0f);

        // 2. 透视除法：将齐次坐标转换为NDC坐标（x/w, y/w, z/w），范围[-1,1]^3
        for (auto& vec : v) vec /= vec.w();

        // 3. 视口变换：将NDC坐标映射到700x700屏幕像素（x/y范围[0,700]）
        for (auto& vert : v)
        {
            vert.x() = 0.5f * width * (vert.x() + 1.0f);  // x：[-1,1] → [0,700]
            vert.y() = 0.5f * height * (vert.y() + 1.0f); // y：[-1,1] → [0,700]
            vert.z() = vert.z() * f1 + f2;                // z：[-1,1] → [0,1]（深度缓冲）
        }

        // 4. 设置三角形顶点数据（三次赋值确保顶点信息正确，避免数据异常）
        for (int j = 0; j < 3; ++j)
        {
            t.setVertex(j, v[j].head<3>()); // 取前3个元素（x,y,z）作为顶点坐标
            t.setVertex(j, v[j].head<3>());
            t.setVertex(j, v[j].head<3>());
        }

        // 5. 设置顶点颜色（红、绿、蓝区分三个顶点，便于观察180°旋转后的顶点位置）
        t.setColor(0, 255.0f, 0.0f, 0.0f);   // 顶点0（原(2,0,-2)）：红色
        t.setColor(1, 0.0f, 255.0f, 0.0f);   // 顶点1（原(0,2,-2)，直角顶点）：绿色
        t.setColor(2, 0.0f, 0.0f, 255.0f);   // 顶点2（原(-2,0,-2)）：蓝色

        // 6. 光栅化三角形线框（绘制三条边，调用Bresenham算法）
        rasterize_wireframe(t);
    }
}

// 光栅化三角形线框：绘制三角形的三条边（a-b、b-c、c-a）
void rst::rasterizer::rasterize_wireframe(const Triangle& t)
{
    draw_line(t.c(), t.a());
    draw_line(t.c(), t.b());
    draw_line(t.b(), t.a());
}

// 设置模型矩阵：更新光栅化器内部的模型变换矩阵
void rst::rasterizer::set_model(const Eigen::Matrix4f& m) { model = m; }

// 设置视图矩阵：更新光栅化器内部的视图变换矩阵
void rst::rasterizer::set_view(const Eigen::Matrix4f& v) { view = v; }

// 设置投影矩阵：更新光栅化器内部的投影变换矩阵
void rst::rasterizer::set_projection(const Eigen::Matrix4f& p) { projection = p; }

// 清空缓冲区：根据参数清空颜色缓冲（黑色）或深度缓冲（无穷大）
void rst::rasterizer::clear(rst::Buffers buff)
{
    if ((buff & rst::Buffers::Color) == rst::Buffers::Color)
    {
        std::fill(frame_buf.begin(), frame_buf.end(), Eigen::Vector3f{ 0, 0, 0 });
    }
    if ((buff & rst::Buffers::Depth) == rst::Buffers::Depth)
    {
        std::fill(depth_buf.begin(), depth_buf.end(), std::numeric_limits<float>::infinity());
    }
}

// 光栅化器构造函数：初始化窗口宽高，分配帧缓冲和深度缓冲内存
rst::rasterizer::rasterizer(int w, int h) : width(w), height(h)
{
    frame_buf.resize(w * h);   // 帧缓冲：存储每个像素的RGB颜色（700*700=490000个元素）
    depth_buf.resize(w * h);   // 深度缓冲：存储每个像素的深度值（与帧缓冲大小一致）
}

// 计算像素索引：将屏幕坐标(x,y)转换为帧缓冲的一维索引（y轴向上，避免翻转）
int rst::rasterizer::get_index(int x, int y)
{
    return (height - y) * width + x;
}

// 设置像素颜色：检查坐标合法性，避免越界访问缓冲（防止程序崩溃）
void rst::rasterizer::set_pixel(const Eigen::Vector3f& point, const Eigen::Vector3f& color)
{
    // 坐标越界判断：x∈[0,700)，y∈[0,700)
    if (point.x() < 0 || point.x() >= width || point.y() < 0 || point.y() >= height) return;

    // 转换为整数坐标并计算索引，赋值颜色
    auto ind = get_index(static_cast<int>(point.x()), static_cast<int>(point.y()));
    frame_buf[ind] = color;
}