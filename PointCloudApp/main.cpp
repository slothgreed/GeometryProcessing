
#include "PointCloudApp.h"
#include "ComputePointCloudApp.h"
#include "ComputeShaderTest.h"
#include "MeshShaderTest.h"

#include <random>

int main()
{
    //KI::ComputeShaderTest app;
	//KI::ComputePointCloudApp app;
	KI::PointCloudApp app;
	//KI::MeshShaderTest app;
	app.Initialize();
	app.Execute();
	app.Finalize();

	return 0;
}

//
//// �G���R�[�h�֐�
//uint32_t Part1By2(uint32_t x)
//{
//    x &= 0x3FF; // 10 �r�b�g�̂ݎg�p
//    x = (x | (x << 16)) & 0x030000FF;
//    x = (x | (x << 8)) & 0x0300F00F;
//    x = (x | (x << 4)) & 0x030C30C3;
//    x = (x | (x << 2)) & 0x09249249;
//    return x;
//}
//
//uint32_t EncodeMorton3D(uint32_t x, uint32_t y, uint32_t z)
//{
//    return (Part1By2(x) | (Part1By2(y) << 1) | (Part1By2(z) << 2));
//}
//
//// �f�R�[�h�֐�
//uint32_t Compact1By2(uint32_t x)
//{
//    x &= 0x09249249;
//    x = (x ^ (x >> 2)) & 0x030C30C3;
//    x = (x ^ (x >> 4)) & 0x0300F00F;
//    x = (x ^ (x >> 8)) & 0x030000FF;
//    x = (x ^ (x >> 16)) & 0x000003FF;
//    return x;
//}
//
//uint32_t DecodeMorton3D_X(uint32_t morton)
//{
//    return Compact1By2(morton);
//}
//
//uint32_t DecodeMorton3D_Y(uint32_t morton)
//{
//    return Compact1By2(morton >> 1);
//}
//
//uint32_t DecodeMorton3D_Z(uint32_t morton)
//{
//    return Compact1By2(morton >> 2);
//}
//
//int main()
//{
//    // ���͍��W
//    uint32_t x = 234, y = 423, z = 345;
//
//    // �G���R�[�h (Morton Code�̐���)
//    uint32_t mortonCode = EncodeMorton3D(x, y, z);
//    std::cout << "Morton Code: " << mortonCode << std::endl;
//
//    // �f�R�[�h (Morton Code������W�̕���)
//    uint32_t decodedX = DecodeMorton3D_X(mortonCode);
//    uint32_t decodedY = DecodeMorton3D_Y(mortonCode);
//    uint32_t decodedZ = DecodeMorton3D_Z(mortonCode);
//
//    // ���ʕ\��
//    std::cout << "Decoded X: " << decodedX << std::endl;
//    std::cout << "Decoded Y: " << decodedY << std::endl;
//    std::cout << "Decoded Z: " << decodedZ << std::endl;
//
//    return 0;
//}

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>


const char* vertexShaderSource = R"(
#version 450
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;

out vec3 vColor;

void main() {
    gl_Position = vec4(aPos, 1.0);
    gl_ViewportIndex = gl_InstanceID; // �e�C���X�^���X���ƂɈقȂ�r���[�|�[�g�ɕ`��
    vColor = aColor;
}
)";

const char* fragmentShaderSource = R"(
#version 430 core
in vec3 vColor;
out vec4 FragColor;

void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

void checkCompileErrors(GLuint shader, std::string type)
{
    GLint success;
    GLchar infoLog[1024];
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n";
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n";
        }
    }
}

int main2()
{
    // GLFW������
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // OpenGL�R���e�L�X�g�ݒ�
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Viewport Array Example", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // �V�F�[�_�[�R���p�C��
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileErrors(vertexShader, "VERTEX");

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileErrors(fragmentShader, "FRAGMENT");

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkCompileErrors(shaderProgram, "PROGRAM");

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // �l�p�`�f�[�^
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // ���� (��)
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // �E�� (��)
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // ��   (��)
    };

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // �r���[�|�[�g�z��ݒ�
    glEnable(GL_SCISSOR_TEST);
    glViewportIndexedf(0, 0, 0, 400, 300);
    glViewportIndexedf(1, 400, 0, 400, 300);
    glScissorIndexed(0, 0, 0, 400, 300);
    glScissorIndexed(1, 400, 0, 400, 300);

    // �`�惋�[�v
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 3, 2); // 2�̃C���X�^���X�ŕ`��

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}