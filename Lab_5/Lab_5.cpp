#include "Shader.h"
#define GLEW_DLL
#define GLFW_DLL

#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "glm.hpp"
#include "matrix_transform.hpp"
#include "type_ptr.hpp"
#include <iostream>
#include "Model.h"

const unsigned int SCR_WIDTH = 1024;
const unsigned int SCR_HEIGHT = 768;

glm::vec3 CameraPos = glm::vec3(0.0, 0.0, 5.0);
glm::vec3 CameraFront = glm::vec3(0.0, 0.0, -1.0);
glm::vec3 CameraUp = glm::vec3(0.0, 1.0, 0.0);

float lastX = SCR_WIDTH / 2;
float lastY = SCR_HEIGHT / 2;
bool firstMouse = true;

float yaw = -90.0f;
float pitch = 0.0f;

glm::mat4 projection = glm::perspective(
    glm::radians(45.0f),
    (float)SCR_WIDTH / (float)SCR_HEIGHT,
    0.1f,
    100.0f);

glm::mat4 view = glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);
glm::mat4 modelMatrix = glm::mat4(1.0f);

// Прототипы функций
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void processInput(GLFWwindow* window);

void settingVec3(Shader& shader, const char* name, const glm::vec3& value) {
    glUniform3fv(glGetUniformLocation(shader.ID, name), 1, &value[0]);
}

void settingFloat(Shader& shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader.ID, name), value);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraSpeed = 2.5f * 0.016f;

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        CameraPos += cameraSpeed * CameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        CameraPos -= cameraSpeed * CameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        CameraPos -= glm::normalize(glm::cross(CameraFront, CameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        CameraPos += glm::normalize(glm::cross(CameraFront, CameraUp)) * cameraSpeed;
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
{
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);

    if (firstMouse)
    {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    CameraFront = glm::normalize(front);
}

int main()
{
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Model Renderer", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glfwSetCursorPosCallback(window, mouse_callback);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        glfwTerminate();
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("vertex_sheder.glsl", "fragment_shader.glsl");
    Model model("xlience.obj");

    // Настройка параметров света (вынесено из цикла рендеринга, так как они постоянны)
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);
    glm::vec3 lightAmbient = lightColor * glm::vec3(0.1f);
    glm::vec3 lightDiffuse = lightColor * glm::vec3(0.8f);
    glm::vec3 lightSpecular = lightColor;
    glm::vec3 lightPosition = glm::vec3(2.0f, 2.0f, 2.0f); // Изменена позиция для лучшего освещения

    // Настройка материала (вынесено из цикла рендеринга)
    glm::vec3 matAmbient = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 matDiffuse = glm::vec3(1.0f, 0.5f, 0.31f);
    glm::vec3 matSpecular = glm::vec3(0.5f, 0.5f, 0.5f);
    float matShininess = 32.0f;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Изменен цвет фона для лучшего отображения освещения
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        view = glm::lookAt(CameraPos, CameraPos + CameraFront, CameraUp);

        shader.use();

        // Передаем матрицы
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setMat4("model", glm::value_ptr(modelMatrix));

        // Передаем параметры света
        settingVec3(shader, "Light_1.ambient", lightAmbient);
        settingVec3(shader, "Light_1.diffuse", lightDiffuse);
        settingVec3(shader, "Light_1.specular", lightSpecular);
        settingVec3(shader, "Light_1.position", lightPosition);

        // Передаем параметры материала
        settingVec3(shader, "Mat_1.ambient", matAmbient);
        settingVec3(shader, "Mat_1.diffuse", matDiffuse);
        settingVec3(shader, "Mat_1.specular", matSpecular);
        settingFloat(shader, "Mat_1.shininess", matShininess);

        // Передаем позицию камеры
        settingVec3(shader, "viewPos", CameraPos);

        // Вычисляем и передаем нормальную матрицу
        glm::mat3 normalMatrix = glm::transpose(glm::inverse(glm::mat3(modelMatrix)));
        shader.setMat3("normalMatrix", normalMatrix);

        model.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
