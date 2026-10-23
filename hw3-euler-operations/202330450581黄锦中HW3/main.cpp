#include "Rendering.h"
#include "EulerOperations.h"

using namespace Rendering;

void init_line_segments()
{
    line_segments.push_back({ { 0.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } });
    line_segments.push_back({ { 1.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 0.0f } });
    line_segments.push_back({ { 1.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } });
    line_segments.push_back({ { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } });


    line_segments.push_back({ { 0.0f, 0.0f, 2.0f }, { 1.0f, 0.0f, 2.0f } });
    line_segments.push_back({ { 1.0f, 0.0f, 2.0f }, { 1.0f, 1.0f, 2.0f } });
    line_segments.push_back({ { 1.0f, 1.0f, 2.0f }, { 0.0f, 1.0f, 2.0f } });
    line_segments.push_back({ { 0.0f, 1.0f, 2.0f }, { 0.0f, 0.0f, 2.0f } });


    line_segments.push_back({ { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 2.0f } });
    line_segments.push_back({ { 1.0f, 0.0f, 0.0f }, { 1.0f, 0.0f, 2.0f } });
    line_segments.push_back({ { 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f, 2.0f } });
    line_segments.push_back({ { 0.0f, 1.0f, 0.0f }, { 0.0f, 1.0f, 2.0f } });
}

void generate_line_segments(Body* _body)
{
    if (!_body)
    {
        return;
    }
    line_segments.clear();
    const auto& edges = _body->edges_;
    for (auto& e : edges)
    {
        auto& p0 = e->he0_->start_vertex_->p_;
        auto& p1 = e->he1_->start_vertex_->p_;
        line_segments.push_back({ { p0[0], p0[1], p0[2] }, { p1[0], p1[1], p1[2] } });
    }

}

void calculate_center_point()
{
    glm::vec3 sum(0.0f);
    int count = 0;

    for (const auto& segment : line_segments)
    {
        sum += segment.start;
        sum += segment.end;
        count += 2;
    }

    center_point = sum / static_cast<float>(count);
}

void calculate_zd()
{
    zd = 0;
    float d;
    for (const auto& segment : line_segments)
    {
        d = glm::length(segment.start - center_point);
        zd = std::max(zd, d);
        d = glm::length(segment.end - center_point);
        zd = std::max(zd, d);
    }

    zd *= 4;
    // std::cout << zd << std::endl;
}

int main()
{
    // 定义欧拉操作类
    EulerOperations eo;

    //构建实体模型
    //构建一个带方形通孔的立方体

    // 1. 创建基础结构
    Vertex* v0 = eo.mvfs(Point(0.0, 0.0, 0.0));
    Loop* base_loop = eo.get_body()->first_face_->first_loop_;

    // 2. 创建底面四边形
    Vertex* v1 = new Vertex(Point(2.0, 0.0, 0.0), nullptr);
    Vertex* v2 = new Vertex(Point(2.0, 2.0, 0.0), nullptr);
    Vertex* v3 = new Vertex(Point(0.0, 2.0, 0.0), nullptr);

    eo.mev(v0, v1, base_loop);
    eo.mev(v1, v2, base_loop);
    eo.mev(v2, v3, base_loop);
    eo.mev(v3, v0, base_loop);

    // 3. 创建顶面四边形
    Vertex* v4 = new Vertex(Point(0.0, 0.0, 2.0), nullptr);
    Vertex* v5 = new Vertex(Point(2.0, 0.0, 2.0), nullptr);
    Vertex* v6 = new Vertex(Point(2.0, 2.0, 2.0), nullptr);
    Vertex* v7 = new Vertex(Point(0.0, 2.0, 2.0), nullptr);

    eo.mev(v0, v4, base_loop);
    eo.mev(v1, v5, base_loop);
    eo.mev(v2, v6, base_loop);
    eo.mev(v3, v7, base_loop);

    eo.mev(v4, v5, base_loop);
    eo.mev(v5, v6, base_loop);
    eo.mev(v6, v7, base_loop);
    eo.mev(v7, v4, base_loop);

    // 4. 创建侧面
    eo.mef(v0, v1, base_loop);  // 底面
    eo.mef(v4, v5, base_loop);  // 顶面
    eo.mef(v0, v4, base_loop);  // 前面
    eo.mef(v1, v5, base_loop);  // 右面
    eo.mef(v2, v6, base_loop);  // 后面
    eo.mef(v3, v7, base_loop);  // 左面

    // 5. 创建孔洞（内环）
    Vertex* hole_v1 = new Vertex(Point(0.7, 0.7, 0.0), nullptr);  // 底面左下
    Vertex* hole_v2 = new Vertex(Point(1.3, 0.7, 0.0), nullptr);  // 底面右下
    Vertex* hole_v3 = new Vertex(Point(1.3, 1.3, 0.0), nullptr);  // 底面右上
    Vertex* hole_v4 = new Vertex(Point(0.7, 1.3, 0.0), nullptr);  // 底面左上

    Vertex* hole_v5 = new Vertex(Point(0.7, 0.7, 2.0), nullptr);  // 顶面左下
    Vertex* hole_v6 = new Vertex(Point(1.3, 0.7, 2.0), nullptr);  // 顶面右下
    Vertex* hole_v7 = new Vertex(Point(1.3, 1.3, 2.0), nullptr);  // 顶面右上
    Vertex* hole_v8 = new Vertex(Point(0.7, 1.3, 2.0), nullptr);  // 顶面左上

    // 查找底面和顶面
    Face* bottom_face = nullptr;
    Face* top_face = nullptr;
    Face* current_face = eo.get_body()->first_face_;

    while (current_face) {
        if (current_face->first_loop_ && current_face->first_loop_->start_he_) {
            Vertex* test_v = current_face->first_loop_->start_he_->start_vertex_;
            if (test_v) {
                if (test_v->p_[2] == 0.0) {
                    bottom_face = current_face;
                }
                else if (test_v->p_[2] == 2.0) {
                    top_face = current_face;
                }
            }
        }
        current_face = current_face->next_face_;
    }

    if (bottom_face && bottom_face->first_loop_ && top_face && top_face->first_loop_) {
        Loop* bottom_loop = bottom_face->first_loop_;
        Loop* top_loop = top_face->first_loop_;

        // 第一步：在底面和顶面分别创建通孔边界
        // 底面通孔边界
        eo.mev(bottom_loop->start_he_->start_vertex_, hole_v1, bottom_loop);
        eo.mev(hole_v1, hole_v2, bottom_loop);
        eo.mev(hole_v2, hole_v3, bottom_loop);
        eo.mev(hole_v3, hole_v4, bottom_loop);

        // 顶面通孔边界
        eo.mev(top_loop->start_he_->start_vertex_, hole_v5, top_loop);
        eo.mev(hole_v5, hole_v6, top_loop);
        eo.mev(hole_v6, hole_v7, top_loop);
        eo.mev(hole_v7, hole_v8, top_loop);

        // 第二步：创建通孔的侧面（四个侧面）
        eo.mev(hole_v1, hole_v5, bottom_loop);  // 前面
        eo.mev(hole_v2, hole_v6, bottom_loop);  // 右面
        eo.mev(hole_v3, hole_v7, bottom_loop);  // 后面
        eo.mev(hole_v4, hole_v8, bottom_loop);  // 左面

        // 第三步：创建内环（在底面和顶面分别创建）
        // 底面孔洞内环
        Loop* bottom_inner_loop = eo.mef(hole_v4, hole_v1, bottom_loop);

        // 顶面孔洞内环
        Loop* top_inner_loop = eo.mef(hole_v8, hole_v5, top_loop);

        if (bottom_inner_loop && top_inner_loop) {

            // 第四步：面融合生成通孔
            eo.kfmrh(bottom_loop, bottom_inner_loop);
            eo.kfmrh(top_loop, top_inner_loop);

            // 清理多余的边
            eo.cleanup_extra_edges();
        }
    }



    // 渲染部分

    // 完成建模后，初始化实体模型的边的数据，并转换为线段，用于渲染
    generate_line_segments(eo.get_body());

    // 注释掉示例初始化函数，使用自己构建的模型
    // init_line_segments();

    calculate_center_point();
    calculate_zd();

    /** --- 以下是渲染代码，以线框模式渲染
    * 按下鼠标左键并移动，可以看到模型的旋转
    * 按下鼠标右键并移动，可以对模型实现平移
    * 按下鼠标中键并移动，可以实现对视图进行缩放
    * 按下键盘 r 键，可以复位原视图
    */

    // 初始化GLFW
    if (!glfwInit())
    {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Euler Operations - Solid Model", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 初始化GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    setup_vertices();

    unsigned int shader_program = create_shader_program();

    projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);

    reset_view();

    glEnable(GL_DEPTH_TEST);

    glLineWidth(2.0f);

    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = glfwGetTime();
        delta_time = currentFrame - last_frame;
        last_frame = currentFrame;

        process_input(window);

        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = glm::lookAt(camera_pos, center_point, camera_up);


        glUseProgram(shader_program);


        glUniformMatrix4fv(glGetUniformLocation(shader_program, "model"), 1, GL_FALSE, glm::value_ptr(model));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "view"), 1, GL_FALSE, glm::value_ptr(view));
        glUniformMatrix4fv(glGetUniformLocation(shader_program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));


        glBindVertexArray(VAO);
        glDrawArrays(GL_LINES, 0, line_segments.size() * 2);


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shader_program);

    glfwTerminate();
    return 0;
}