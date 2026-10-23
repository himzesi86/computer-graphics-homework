#ifndef _RENDERING_H_
#define _RENDERING_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>


namespace Rendering
{

    // 线段结构体
    struct LineSegment
    {
        glm::vec3 start;
        glm::vec3 end;
    };

    // 全局变量
    std::vector<LineSegment> line_segments;
    glm::vec3 center_point;
    unsigned int VAO, VBO;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    glm::mat4 projection = glm::mat4(1.0f);

    // 窗口尺寸
    const unsigned int SCR_WIDTH = 800;
    const unsigned int SCR_HEIGHT = 600;

    // 摄像机参数
    float zd = 20.0f;
    glm::vec3 camera_pos = glm::vec3(0.0f, 0.0f, zd);
    glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);

    // 鼠标交互参数
    bool first_mouse = true;
    float last_x = SCR_WIDTH / 2.0f;
    float last_y = SCR_HEIGHT / 2.0f;
    bool mouse_left_pressed = false;
    bool mouse_right_pressed = false;
    bool mouse_middle_pressed = false;
    float delta_time = 0.0f;
    float last_frame = 0.0f;

    // 初始化顶点数据
    void setup_vertices()
    {
        std::vector<float> vertices;

        for (const auto& segment : line_segments)
        {
            vertices.push_back(segment.start.x);
            vertices.push_back(segment.start.y);
            vertices.push_back(segment.start.z);

            vertices.push_back(segment.end.x);
            vertices.push_back(segment.end.y);
            vertices.push_back(segment.end.z);
        }

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    // 顶点着色器
    const char* vertex_shader_source = R"(
                                            #version 330 core
                                            layout (location = 0) in vec3 aPos;

                                            uniform mat4 model;
                                            uniform mat4 view;
                                            uniform mat4 projection;

                                            void main()
                                            {
                                                gl_Position = projection * view * model * vec4(aPos, 1.0);
                                            }
                                            )";

    // 片段着色器
    const char* fragment_shader_source = R"(
                                            #version 330 core
                                            out vec4 FragColor;

                                            void main()
                                            {
                                                FragColor = vec4(1.0, 0.0, 0.0, 1.0); // 红色
                                            }
                                            )";

    // 编译着色器
    unsigned int compile_shader(const char* source, GLenum type)
    {
        unsigned int shader = glCreateShader(type);
        glShaderSource(shader, 1, &source, NULL);
        glCompileShader(shader);

        int success;
        char info_log[512];
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

        if (!success)
        {
            glGetShaderInfoLog(shader, 512, NULL, info_log);
            std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << info_log << std::endl;
        }

        return shader;
    }

    // 创建着色器程序
    unsigned int create_shader_program()
    {
        unsigned int vertex_shader = compile_shader(vertex_shader_source, GL_VERTEX_SHADER);
        unsigned int fragment_shader = compile_shader(fragment_shader_source, GL_FRAGMENT_SHADER);

        unsigned int shader_program = glCreateProgram();
        glAttachShader(shader_program, vertex_shader);
        glAttachShader(shader_program, fragment_shader);
        glLinkProgram(shader_program);

        int success;
        char infoLog[512];
        glGetProgramiv(shader_program, GL_LINK_STATUS, &success);

        if (!success)
        {
            glGetProgramInfoLog(shader_program, 512, NULL, infoLog);
            std::cout << "ERROR::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        }

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        return shader_program;
    }

    // 重置视图
    void reset_view()
    {
        model = glm::mat4(1.0f);
        camera_pos = glm::vec3(center_point.x, center_point.y, center_point.z + zd);
        camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
        camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    // 鼠标位置回调
    void mouse_callback(GLFWwindow* window, double xpos, double ypos)
    {
        if (first_mouse)
        {
            last_x = xpos;
            last_y = ypos;
            first_mouse = false;
        }

        float dx;
        float dy;
        dx = xpos - last_x;
        dy = ypos - last_y;

        float angle = std::sqrt(dx * dx + dy * dy) * 0.01;

        float axis_x = dy;
        float axis_y = -dx;
        float axis_z = 0.0f;

        float len = std::sqrt(axis_x * axis_x + axis_y * axis_y);
        if (len != 0)
        {
            axis_x /= len;
            axis_y /= len;
            axis_z /= len;
        }

        float xoffset = xpos - last_x;
        float yoffset = last_y - ypos;
        last_x = xpos;
        last_y = ypos;

        if (mouse_left_pressed)
        {
            // 跟踪球旋转
            // float sensitivity = 0.005f;
            // xoffset *= sensitivity;
            // yoffset *= sensitivity;

            // 绕Y轴旋转（水平移动）
            // model = glm::rotate(model, xoffset, glm::vec3(0.0f, 1.0f, 0.0f));
            // 绕X轴旋转（垂直移动）
            // model = glm::rotate(model, yoffset, glm::vec3(1.0f, 0.0f, 0.0f));

            model = glm::translate(model, glm::vec3(center_point[0], center_point[1], center_point[2]));
            model = glm::rotate(model, angle, glm::vec3(axis_x, axis_y, axis_z));
            model = glm::translate(model, glm::vec3(-center_point[0], -center_point[1], -center_point[2]));
        }

        if (mouse_right_pressed)
        {
            // 平移
            float sensitivity = 0.01f;
            // xoffset *= sensitivity;
            // yoffset *= sensitivity;

            // model = glm::translate(model, glm::vec3(xoffset, -yoffset, 0.0f));
            // model = glm::inverse(model);
         
            model = glm::translate(model, glm::vec3(dx * sensitivity, -dy * sensitivity, 0.0));
        }

        if (mouse_middle_pressed)
        {
            camera_pos += camera_front * dy * 0.05f;
        }



    }

    // 鼠标按钮回调
    void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
    {
        if (button == GLFW_MOUSE_BUTTON_LEFT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_left_pressed = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_left_pressed = false;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_RIGHT)
        {
            if (action == GLFW_PRESS)
            {
                mouse_right_pressed = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_right_pressed = false;
            }
        }

        if (button == GLFW_MOUSE_BUTTON_MIDDLE)
        {
            if (action == GLFW_PRESS)
            {
                mouse_middle_pressed = true;
            }
            else if (action == GLFW_RELEASE)
            {
                mouse_middle_pressed = false;
            }

        }
    }

    // 键盘输入处理
    void process_input(GLFWwindow* window)
    {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        {
            glfwSetWindowShouldClose(window, true);
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            reset_view();
        }

        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
        {
            // 缩小效果
            camera_pos += camera_front * 1.0f * 0.1f;
        }

        if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
        {
            // 放大效果
            camera_pos -= camera_front * 1.0f * 0.1f;
        }
    }

    // 窗口大小调整回调
    void framebuffer_size_callback(GLFWwindow* window, int width, int height)
    {
        glViewport(0, 0, width, height);
    }


    //-------------------------------------------------------------------------------------------------
} // end of namespace Rendering

#endif