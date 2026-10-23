#include "Triangle.hpp"
#include "rasterizer.hpp"
#include <Eigen/Eigen>
#include <iostream>
#include <opencv2/opencv.hpp>
#include <cmath>

// 圆周率常量定义
constexpr double MY_PI = 3.1415926;

// 生成视图矩阵：将相机位置反向平移，使世界原点与相机原点重合
Eigen::Matrix4f get_view_matrix(Eigen::Vector3f eye_pos)
{
    Eigen::Matrix4f view = Eigen::Matrix4f::Identity();
    Eigen::Matrix4f translate;
    translate << 1, 0, 0, -eye_pos[0],
        0, 1, 0, -eye_pos[1],
        0, 0, 1, -eye_pos[2],
        0, 0, 0, 1;
    view = translate * view;
    return view;
}

// 生成模型矩阵：实现绕Z轴旋转（支持任意角度输入）
Eigen::Matrix4f get_model_matrix(float rotation_angle)
{
    Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
    // 角度转弧度（三角函数需弧度制输入）
    float radian = rotation_angle * MY_PI / 180.0f;
    float cos_theta = cos(radian);
    float sin_theta = sin(radian);

    // 绕Z轴旋转矩阵：x'=xcosθ-ysinθ, y'=xsinθ+ycosθ, z'=z
    model << cos_theta, -sin_theta, 0, 0,
        sin_theta, cos_theta, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;
    return model;
}

// 生成透视投影矩阵：固定参数确保投影无变形
Eigen::Matrix4f get_projection_matrix(float eye_fov, float aspect_ratio,
    float zNear, float zFar)
{
    Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

    // 1. 透视转正交矩阵：挤压透视锥体为长方体
    Eigen::Matrix4f persp_to_ortho;
    persp_to_ortho << zNear, 0, 0, 0,
        0, zNear, 0, 0,
        0, 0, zNear + zFar, -zNear * zFar,
        0, 0, 1, 0;

    // 2. 计算正交投影边界：基于视场角和宽高比确定近裁剪面范围
    float fov_radian = eye_fov * MY_PI / 180.0f;
    float half_fov = fov_radian / 2.0f;
    float top = abs(zNear) * tan(half_fov);
    float bottom = -top;
    float right = top * aspect_ratio;
    float left = -right;

    // 3. 正交投影矩阵：平移（中心到原点）+ 缩放（映射到[-1,1]）
    Eigen::Matrix4f ortho_translate;
    ortho_translate << 1, 0, 0, -(left + right) / 2,
        0, 1, 0, -(top + bottom) / 2,
        0, 0, 1, -(zNear + zFar) / 2,
        0, 0, 0, 1;

    Eigen::Matrix4f ortho_scale;
    ortho_scale << 2 / (right - left), 0, 0, 0,
        0, 2 / (top - bottom), 0, 0,
        0, 0, 2 / (zNear - zFar), 0,
        0, 0, 0, 1;

    Eigen::Matrix4f ortho = ortho_scale * ortho_translate;

    // 4. 最终投影矩阵：正交矩阵 * 透视转正交矩阵
    projection = ortho * persp_to_ortho;
    return projection;
}

// 【提高项】绕任意过原点轴旋转矩阵（基于罗德里格斯公式）
Eigen::Matrix4f get_rotation(Vector3f axis, float angle)
{
    Eigen::Matrix4f rotation = Eigen::Matrix4f::Identity();
    // 轴向量归一化
    axis.normalize();
    float x = axis.x();
    float y = axis.y();
    float z = axis.z();

    // 角度转弧度
    float radian = angle * MY_PI / 180.0f;
    float cos_theta = cos(radian);
    float sin_theta = sin(radian);
    float one_minus_cos = 1 - cos_theta;

    // 罗德里格斯旋转公式构建矩阵
    rotation <<
        cos_theta + x * x * one_minus_cos, x* y* one_minus_cos - z * sin_theta, x* z* one_minus_cos + y * sin_theta, 0,
        y* x* one_minus_cos + z * sin_theta, cos_theta + y * y * one_minus_cos, y* z* one_minus_cos - x * sin_theta, 0,
        z* x* one_minus_cos - y * sin_theta, z* y* one_minus_cos + x * sin_theta, cos_theta + z * z * one_minus_cos, 0,
        0, 0, 0, 1;
    return rotation;
}

int main(int argc, const char** argv)
{
    // 关键修改：初始旋转角度设为180°，确保程序启动时三角形已绕Z轴旋转180°
    float angle = 180.0f;
    bool command_line = false;
    std::string filename = "output.png";

    // 命令行参数处理：支持指定旋转角度和输出图片（命令行参数优先级高于默认180°）
    if (argc >= 3) {
        command_line = true;
        angle = std::stof(argv[2]); // 若命令行指定角度（如 -r 90），则覆盖默认180°
        if (argc == 4) {
            filename = std::string(argv[3]);
        }
        else {
            std::cerr << "参数错误：正确格式 HWd.exe -r 角度 [输出文件名]" << std::endl;
            return 1;
        }
    }

    // 初始化光栅化器（窗口700x700）
    rst::rasterizer r(700, 700);
    Eigen::Vector3f eye_pos = { 0, 0, 5 }; // 相机位置（z轴正方向，距离原点5单位）

    // 三角形顶点（固定直角三角形：直角顶点在(0,2,-2)，两直角边沿x轴正负、y轴负方向）
    std::vector<Eigen::Vector3f> pos{ {2, 0, -2}, {0, 2, -2}, {-2, 0, -2} };
    std::vector<Eigen::Vector3i> ind{ {0, 1, 2} }; // 顶点索引（构成三角形）

    // 加载顶点和索引到光栅化器
    auto pos_id = r.load_positions(pos);
    auto ind_id = r.load_indices(ind);

    int key = 0;
    int frame_count = 0;

    // 命令行模式：生成指定角度的图片并保存
    if (command_line) {
        r.clear(rst::Buffers::Color | rst::Buffers::Depth); // 清空颜色和深度缓冲区
        r.set_model(get_model_matrix(angle)); // 应用旋转矩阵（默认180°或命令行指定角度）
        r.set_view(get_view_matrix(eye_pos)); // 设置视图矩阵
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50)); // 45°视场，1:1宽高比
        r.draw(pos_id, ind_id, rst::Primitive::Triangle); // 绘制三角形

        // 转换帧缓冲区数据为OpenCV图像并保存
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f); // 从32位浮点型转8位无符号整型
        cv::imwrite(filename, image);
        std::cout << "图片已保存至：" << filename << std::endl;
        return 0;
    }

    // 交互模式：A/D键控制旋转，Esc键退出
    while (key != 27) { // 27为Esc键ASCII码
        r.clear(rst::Buffers::Color | rst::Buffers::Depth); // 每帧清空缓冲区
        r.set_model(get_model_matrix(angle));
        r.set_view(get_view_matrix(eye_pos));
        r.set_projection(get_projection_matrix(45, 1, 0.1, 50));
        r.draw(pos_id, ind_id, rst::Primitive::Triangle);

        // 显示图像
        cv::Mat image(700, 700, CV_32FC3, r.frame_buffer().data());
        image.convertTo(image, CV_8UC3, 1.0f);
        cv::imshow("Rotation & Projection (Initial 180°)", image); // 窗口标题标注初始角度
        key = cv::waitKey(10); // 等待10ms检测键盘输入

        // 输出帧计数和当前角度（调试用）
        std::cout << "帧计数: " << frame_count++ << " | 当前角度: " << angle << "°" << std::endl;

        // 键盘控制逻辑：A键顺时针旋转（角度+10°），D键逆时针旋转（角度-10°）
        if (key == 'a' || key == 'A') angle += 10;
        else if (key == 'd' || key == 'D') angle -= 10;
    }

    cv::destroyAllWindows(); // 关闭所有OpenCV窗口
    return 0;
}