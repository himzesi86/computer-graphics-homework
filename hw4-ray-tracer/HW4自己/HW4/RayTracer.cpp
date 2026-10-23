#include "RayTracer.h"
#include <iostream>

Intersection Sphere::intersect(const ray& ray_data) const
{
    Intersection result;
    Vector3f oc = ray_data.origin - center;
    float a = ray_data.direction.dot(ray_data.direction);
    float b = 2.0f * oc.dot(ray_data.direction);
    float c = oc.dot(oc) - radius * radius;
    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0)
    {
        float sqrt_discriminant = sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);

        float t = min(t1, t2);
        if (t < 0.001f)
        {
            t = max(t1, t2);
        }

        if (t > 0.001f && t < result.distance)
        {
            result.hit = true;
            result.distance = t;
            result.point = ray_data.origin + t * ray_data.direction;
            result.normal = (result.point - center).normalized();
            result.material_data = material_data;
        }
    }

    return result;
}

Intersection Cube::intersect(const ray& ray_data) const
{
    Intersection result;

    Vector3f t1 = (min_point - ray_data.origin).array() / ray_data.direction.array();
    Vector3f t2 = (max_point - ray_data.origin).array() / ray_data.direction.array();

    Vector3f t_min = t1.cwiseMin(t2);
    Vector3f t_max = t1.cwiseMax(t2);

    float t_near = max(max(t_min.x(), t_min.y()), t_min.z());
    float t_far = min(min(t_max.x(), t_max.y()), t_max.z());

    if (t_near > t_far || t_far < 0.001f)
    {
        return result;
    }

    float t = (t_near > 0.001f) ? t_near : t_far;

    if (t > 0.001f && t < result.distance)
    {
        result.hit = true;
        result.distance = t;
        result.point = ray_data.origin + t * ray_data.direction;

        // 计算法向量
        const float epsilon = 0.0001f;
        if (fabs(result.point.x() - min_point.x()) < epsilon)
        {
            result.normal = Vector3f(-1, 0, 0);
        }
        else if (fabs(result.point.x() - max_point.x()) < epsilon)
        {
            result.normal = Vector3f(1, 0, 0);
        }
        else if (fabs(result.point.y() - min_point.y()) < epsilon)
        {
            result.normal = Vector3f(0, -1, 0);
        }
        else if (fabs(result.point.y() - max_point.y()) < epsilon)
        {
            result.normal = Vector3f(0, 1, 0);
        }
        else if (fabs(result.point.z() - min_point.z()) < epsilon)
        {
            result.normal = Vector3f(0, 0, -1);
        }
        else
        {
            result.normal = Vector3f(0, 0, 1);
        }

        result.material_data = material_data;
    }

    return result;
}

Intersection plane::intersect(const ray& ray_data) const
{
    Intersection result;
    float denom = normal.dot(ray_data.direction);

    if (fabs(denom) > 0.0001f)
    {
        float t = (point - ray_data.origin).dot(normal) / denom;

        if (t > 0.001f && t < result.distance)
        {
            result.hit = true;
            result.distance = t;
            result.point = ray_data.origin + t * ray_data.direction;
            result.normal = normal;
            result.material_data = material_data;
        }
    }

    return result;
}

// 圆柱体相交检测实现
Intersection Cylinder::intersect(const ray& ray_data) const
{
    Intersection result;

    Vector3f base_to_origin = ray_data.origin - base_center;
    Vector3f axis_normalized = axis.normalized();

    // 计算射线方向在圆柱轴向上的投影
    float dot_dir_axis = ray_data.direction.dot(axis_normalized);
    float dot_origin_axis = base_to_origin.dot(axis_normalized);

    // 计算射线方向在垂直于轴向上的分量
    Vector3f dir_perp = ray_data.direction - dot_dir_axis * axis_normalized;
    Vector3f origin_perp = base_to_origin - dot_origin_axis * axis_normalized;

    // 解二次方程
    float a = dir_perp.dot(dir_perp);
    float b = 2.0f * origin_perp.dot(dir_perp);
    float c = origin_perp.dot(origin_perp) - radius * radius;

    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0)
    {
        float sqrt_discriminant = sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);

        float t = min(t1, t2);
        if (t < 0.001f)
        {
            t = max(t1, t2);
        }

        if (t > 0.001f && t < result.distance)
        {
            Vector3f intersection_point = ray_data.origin + t * ray_data.direction;
            Vector3f base_to_intersection = intersection_point - base_center;
            float height_along_axis = base_to_intersection.dot(axis_normalized);

            // 检查交点是否在圆柱高度范围内
            if (height_along_axis >= 0 && height_along_axis <= height)
            {
                result.hit = true;
                result.distance = t;
                result.point = intersection_point;

                // 计算法向量（垂直于轴线）
                Vector3f center_proj = base_center + height_along_axis * axis_normalized;
                result.normal = (intersection_point - center_proj).normalized();
                result.material_data = material_data;
            }
        }
    }

    return result;
}

// 圆环体相交检测实现（简化版本）
Intersection Torus::intersect(const ray& ray_data) const
{
    Intersection result;

    // 简化实现：将圆环近似为球体进行相交检测
    Vector3f oc = ray_data.origin - center;
    float a = ray_data.direction.dot(ray_data.direction);
    float b = 2.0f * oc.dot(ray_data.direction);
    float c = oc.dot(oc) - (major_radius + minor_radius) * (major_radius + minor_radius);
    float discriminant = b * b - 4 * a * c;

    if (discriminant >= 0)
    {
        float sqrt_discriminant = sqrt(discriminant);
        float t1 = (-b - sqrt_discriminant) / (2.0f * a);
        float t2 = (-b + sqrt_discriminant) / (2.0f * a);

        float t = min(t1, t2);
        if (t < 0.001f)
        {
            t = max(t1, t2);
        }

        if (t > 0.001f && t < result.distance)
        {
            result.hit = true;
            result.distance = t;
            result.point = ray_data.origin + t * ray_data.direction;
            result.normal = (result.point - center).normalized();
            result.material_data = material_data;
        }
    }

    return result;
}

Intersection Scene::find_closest_intersection(const ray& ray_data) const
{
    Intersection closest;

    for (const auto& obj : objects)
    {
        Intersection current_intersection = obj->intersect(ray_data);

        if (current_intersection.hit && current_intersection.distance < closest.distance)
        {
            closest = current_intersection;
        }
    }

    return closest;
}

// 计算反射方向
Vector3f reflect(const Vector3f& incident, const Vector3f& normal)
{
    return incident - 2 * incident.dot(normal) * normal;
}

// 计算折射方向（使用斯涅尔定律）
bool refract(const Vector3f& incident, const Vector3f& normal, float n1, float n2, Vector3f& refracted)
{
    Vector3f incident_norm = incident.normalized();
    float cos_theta1 = -incident_norm.dot(normal);
    float n = n1 / n2;
    float sin_theta2_sq = n * n * (1.0f - cos_theta1 * cos_theta1);

    // 检查全内反射
    if (sin_theta2_sq > 1.0f)
    {
        return false; // 全内反射，无折射
    }

    float cos_theta2 = sqrt(1.0f - sin_theta2_sq);
    refracted = n * incident_norm + (n * cos_theta1 - cos_theta2) * normal;
    return true;
}

// 计算菲涅尔反射系数
float fresnel(const Vector3f& incident, const Vector3f& normal, float n1, float n2)
{
    float cos_theta1 = -incident.normalized().dot(normal);
    float n = n1 / n2;
    float sin_theta2_sq = n * n * (1.0f - cos_theta1 * cos_theta1);

    if (sin_theta2_sq > 1.0f)
    {
        return 1.0f; // 全内反射
    }

    float cos_theta2 = sqrt(1.0f - sin_theta2_sq);
    float r_parallel = (n2 * cos_theta1 - n1 * cos_theta2) / (n2 * cos_theta1 + n1 * cos_theta2);
    float r_perpendicular = (n1 * cos_theta1 - n2 * cos_theta2) / (n1 * cos_theta1 + n2 * cos_theta2);

    return (r_parallel * r_parallel + r_perpendicular * r_perpendicular) / 2.0f;
}

/** 递归实现一条光线跟踪，返回该射线对应的像素的颜色值 */
Vector3f trace_ray(const ray& ray_data, const Scene& scene_data, int depth)
{
    // 每递归调用一次，深度减少1
    --depth;

    // 深度小于0，则返回黑色
    if (depth <= 0)
    {
        return Vector3f(0, 0, 0);
    }

    Vector3f final_color(0, 0, 0);

    // 寻找最近的交点
    Intersection intersection = scene_data.find_closest_intersection(ray_data);

    // 如果没有交点，返回背景色
    if (!intersection.hit)
    {
        return scene_data.background_color;
    }

    // 计算局部光照（I_c）
    Vector3f local_color(0, 0, 0);
    Vector3f intersection_point = intersection.point;
    Vector3f normal = intersection.normal;
    Material material = intersection.material_data;

    // 添加环境光
    local_color += material.diffuse.cwiseProduct(scene_data.ambient_light);

    // 对每个光源计算光照
    for (const auto& light : scene_data.lights)
    {
        Vector3f light_dir = (light.position - intersection_point).normalized();
        float distance_to_light = (light.position - intersection_point).norm();

        // 阴影检测：从交点向光源发射射线
        ray shadow_ray(intersection_point + normal * 0.001f, light_dir);
        Intersection shadow_intersection = scene_data.find_closest_intersection(shadow_ray);

        // 如果阴影射线击中物体且距离小于到光源的距离，则该点在阴影中
        if (shadow_intersection.hit && shadow_intersection.distance < distance_to_light)
        {
            continue; // 跳过这个光源的贡献
        }

        // 漫反射计算
        float diffuse_intensity = max(0.0f, normal.dot(light_dir));
        Vector3f diffuse = material.diffuse.cwiseProduct(light.color) * diffuse_intensity * light.intensity;

        // 镜面反射计算（Blinn-Phong模型）
        Vector3f view_dir = -ray_data.direction.normalized();
        Vector3f half_dir = (light_dir + view_dir).normalized();
        float specular_intensity = pow(max(0.0f, normal.dot(half_dir)), material.shininess);
        Vector3f specular = material.specular.cwiseProduct(light.color) * specular_intensity * light.intensity;

        // 光照衰减（简单线性衰减）
        float attenuation = 1.0f / (1.0f + 0.1f * distance_to_light); // 恢复原衰减系数

        local_color += (diffuse + specular) * attenuation;
    }

    // 添加自发光
    local_color += material.emission;

    // 计算反射光（I_r）
    Vector3f reflected_color(0, 0, 0);
    if (material.reflectivity > 0.0f)
    {
        Vector3f reflected_dir = reflect(ray_data.direction, normal);
        ray reflected_ray(intersection_point + normal * 0.001f, reflected_dir);
        reflected_color = trace_ray(reflected_ray, scene_data, depth);
    }

    // 计算折射光（I_t）
    Vector3f refracted_color(0, 0, 0);
    if (material.transparency > 0.0f)
    {
        // 确定折射率（假设外部为空气，折射率为1.0）
        float n1 = 1.0f; // 空气折射率
        float n2 = material.refractive_index;

        // 检查光线是从外部进入还是内部射出
        float cos_theta = -ray_data.direction.normalized().dot(normal);
        if (cos_theta < 0)
        {
            // 从内部射出，翻转法线和折射率
            normal = -normal;
            swap(n1, n2);
            cos_theta = -cos_theta;
        }

        Vector3f refracted_dir;
        if (refract(ray_data.direction, normal, n1, n2, refracted_dir))
        {
            ray refracted_ray(intersection_point - normal * 0.001f, refracted_dir);
            refracted_color = trace_ray(refracted_ray, scene_data, depth);
        }
        else
        {
            // 全内反射，当作反射处理
            Vector3f reflected_dir = reflect(ray_data.direction, normal);
            ray reflected_ray(intersection_point + normal * 0.001f, reflected_dir);
            refracted_color = trace_ray(reflected_ray, scene_data, depth);
        }
    }

    // 合并颜色：I = I_c + K_s * I_r + K_t * I_t
    final_color = local_color +
        material.reflectivity * reflected_color +
        material.transparency * refracted_color;

    // 调整整体亮度（降低亮度系数）
    final_color *= 1.1f;

    return final_color.cwiseMin(Vector3f(1, 1, 1)).cwiseMax(Vector3f(0, 0, 0));
}

/** 对整个场景的每个像素计算颜色值 */
void render_scene(const Scene& scene_data, Mat& image, int width, int height, int max_depth)
{
    // 调整相机位置，确保能看到所有物体
    Vector3f camera_pos(0, 6, -10);  // 相机在房间前方稍高的位置
    Vector3f look_at(0, 0, 10);      // 看向房间深处
    Vector3f up_dir(0, 1, 0);

    Vector3f forward = (look_at - camera_pos).normalized();
    Vector3f right = forward.cross(up_dir).normalized();
    Vector3f actual_up = right.cross(forward);

    float fov = 60.0f * M_PI / 180.0f;  // 增大视野
    float aspect_ratio = float(width) / float(height);
    float scale = tan(fov * 0.5f);

    // 对每个像素进行计算
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            float pixel_x = (2.0f * (x + 0.5f) / width - 1.0f) * aspect_ratio * scale;
            float pixel_y = (1.0f - 2.0f * (y + 0.5f) / height) * scale;

            Vector3f ray_dir = (forward + right * pixel_x + actual_up * pixel_y).normalized();
            ray current_ray(camera_pos, ray_dir);

            Vector3f pixel_color = trace_ray(current_ray, scene_data, max_depth);

            // OpenCV使用BGR格式，需要转换
            image.at<Vec3f>(y, x) = Vec3f(pixel_color[2], pixel_color[1], pixel_color[0]);
        }

        // 显示进度
        if (y % 50 == 0)
        {
            cout << "渲染进度: " << (y * 100 / height) << "%" << endl;
        }
    }
}