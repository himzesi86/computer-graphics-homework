#include "RayTracer.h"
#include <iostream>

void setup_scene(Scene& scene_data);

void setup_scene(Scene& scene_data)
{
    // 调整环境光亮度（降低亮度）
    scene_data.ambient_light = Vector3f(0.15f, 0.15f, 0.15f);
    scene_data.background_color = Vector3f(0.1f, 0.1f, 0.1f);

    //--- 构建房间场景 ---//
    auto floor_plane = make_shared<plane>(Vector3f(0, -5, 0), Vector3f(0, 1, 0));
    floor_plane->material_data.diffuse = Vector3f(0.3f, 0.5f, 0.2f);  // 恢复原亮度
    floor_plane->material_data.shininess = 16.0f;
    scene_data.add_object(floor_plane);

    auto ceiling_plane = make_shared<plane>(Vector3f(0, 15, 0), Vector3f(0, -1, 0));
    ceiling_plane->material_data.diffuse = Vector3f(0.5f, 0.5f, 0.5f);  // 恢复原亮度
    scene_data.add_object(ceiling_plane);

    auto left_wall = make_shared<plane>(Vector3f(-15, 0, 0), Vector3f(1, 0, 0));
    left_wall->material_data.diffuse = Vector3f(0.7f, 0.3f, 0.3f);  // 恢复原亮度
    scene_data.add_object(left_wall);

    auto right_wall = make_shared<plane>(Vector3f(15, 0, 0), Vector3f(-1, 0, 0));
    right_wall->material_data.diffuse = Vector3f(0.3f, 0.5f, 0.7f);  // 恢复原亮度
    scene_data.add_object(right_wall);

    auto front_wall = make_shared<plane>(Vector3f(0, 0, 20), Vector3f(0, 0, -1));
    front_wall->material_data.diffuse = Vector3f(0.6f, 0.1f, 0.3f);  // 恢复原亮度
    scene_data.add_object(front_wall);

    auto back_wall = make_shared<plane>(Vector3f(0, 0, -20), Vector3f(0, 0, 1));
    back_wall->material_data.diffuse = Vector3f(0.6f, 0.6f, 0.1f);  // 恢复原亮度
    scene_data.add_object(back_wall);

    //--- 新增物体（放在原有物体前方）---//

    //--- 新增：圆柱体（放在玻璃球前方）
    auto red_cylinder = make_shared<Cylinder>(Vector3f(0, 0, 2), Vector3f(0, 1, 0), 1.0f, 2.5f);
    red_cylinder->material_data.diffuse = Vector3f(0.8f, 0.2f, 0.2f);  // 红色
    red_cylinder->material_data.specular = Vector3f(0.5f, 0.5f, 0.5f);
    red_cylinder->material_data.shininess = 48.0f;
    red_cylinder->material_data.reflectivity = 0.1f;
    scene_data.add_object(red_cylinder);

    //--- 新增：圆环体（放在绿色球前方）
    auto purple_torus = make_shared<Torus>(Vector3f(-6, 0, 6), Vector3f(0, 1, 0), 1.5f, 0.5f);
    purple_torus->material_data.diffuse = Vector3f(0.6f, 0.2f, 0.7f);  // 紫色
    purple_torus->material_data.specular = Vector3f(0.6f, 0.6f, 0.6f);
    purple_torus->material_data.shininess = 96.0f;
    purple_torus->material_data.reflectivity = 0.3f;
    scene_data.add_object(purple_torus);

    //--- 新增：小型圆柱体（放在立方体前方）
    auto yellow_cylinder = make_shared<Cylinder>(Vector3f(6, 0, 4), Vector3f(0, 1, 0), 0.8f, 1.5f);
    yellow_cylinder->material_data.diffuse = Vector3f(0.8f, 0.8f, 0.2f);  // 黄色
    yellow_cylinder->material_data.specular = Vector3f(0.4f, 0.4f, 0.4f);
    yellow_cylinder->material_data.shininess = 32.0f;
    yellow_cylinder->material_data.reflectivity = 0.1f;
    scene_data.add_object(yellow_cylinder);

    //--- 放置原有模型（放在新物体后方）---//

    //--- 玻璃球（中间，放在圆柱体后方）
    auto glass_sphere = make_shared<Sphere>(Vector3f(0, 0, 5), 2.0f);
    glass_sphere->material_data.diffuse = Vector3f(0.1f, 0.1f, 0.1f);
    glass_sphere->material_data.specular = Vector3f(1.0f, 1.0f, 1.0f);
    glass_sphere->material_data.transparency = 0.8f;
    glass_sphere->material_data.reflectivity = 0.2f;
    glass_sphere->material_data.refractive_index = 1.5f;
    glass_sphere->material_data.shininess = 128.0f;
    scene_data.add_object(glass_sphere);

    //--- 绿色球（放在圆环体后方）
    auto green_sphere = make_shared<Sphere>(Vector3f(-6, 0, 8), 2.0f);
    green_sphere->material_data.diffuse = Vector3f(0.2f, 0.8f, 0.2f);  // 恢复原亮度
    green_sphere->material_data.specular = Vector3f(0.5f, 0.5f, 0.5f);  // 恢复原亮度
    green_sphere->material_data.shininess = 64.0f;
    green_sphere->material_data.reflectivity = 0.3f;
    scene_data.add_object(green_sphere);

    //--- 立方体（放在小型圆柱体后方）
    auto blue_cube = make_shared<Cube>(Vector3f(4, -2, 6), Vector3f(8, 2, 10));
    blue_cube->material_data.diffuse = Vector3f(0.2f, 0.6f, 0.8f);  // 恢复原亮度
    blue_cube->material_data.specular = Vector3f(0.4f, 0.4f, 0.4f);  // 恢复原亮度
    blue_cube->material_data.shininess = 32.0f;
    blue_cube->material_data.reflectivity = 0.2f;
    scene_data.add_object(blue_cube);

    //--- 设置光源（降低亮度）---//
    scene_data.add_light(Light(Vector3f(0, 14, 5), Vector3f(1.0f, 1.0f, 1.0f), 1.0f));  // 恢复原强度
    scene_data.add_light(Light(Vector3f(-10, 10, 0), Vector3f(0.9f, 0.9f, 0.7f), 0.7f)); // 恢复原强度
    scene_data.add_light(Light(Vector3f(10, 10, 0), Vector3f(0.2f, 0.9f, 0.9f), 0.7f));  // 恢复原强度
}

int main()
{
    cout << "开始初始化场景..." << endl;
    Scene main_scene;
    setup_scene(main_scene);
    cout << "场景初始化完成" << endl;

    int image_width = 800;  // 渲染图片的宽
    int image_height = 600;  // 渲染图片的高
    int max_ray_depth = 5;    // 设置最大递归深度

    cout << "开始渲染图像..." << endl;
    cout << "图像尺寸: " << image_width << " x " << image_height << endl;
    cout << "最大递归深度: " << max_ray_depth << endl;

    Mat output_image(image_height, image_width, CV_32FC3, Scalar(0, 0, 0));

    //--- 渲染场景 ---//
    render_scene(main_scene, output_image, image_width, image_height, max_ray_depth);

    Mat display_image;
    output_image.convertTo(display_image, CV_8UC3, 255.0);

    cout << "渲染完成，显示图像..." << endl;
    imshow("光线跟踪渲染结果", display_image);
    waitKey(0);

    // 保存图像
    imwrite("ray_tracing_result.jpg", display_image);
    cout << "图像已保存为 ray_tracing_result.jpg" << endl;

    return 0;
}