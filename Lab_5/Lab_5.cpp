#include "Shader.h"
#include "Model.h"
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <matrix_transform.hpp>
#include <type_ptr.hpp>
#include <iostream>

const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;
float yaw = -90.0f;
float pitch = 0.0f;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct ObjectTransform {
    glm::vec3 position = glm::vec3(0.0f);

    // Ограничения движения
    struct {
        float min = -3.0f;
        float max = 3.0f;
    } yLimit; // Для объекта 1 (Y-axis)

    struct {
        float min = -5.0f;
        float max = 5.0f;
    } xLimit; // Для объекта 2 (X-axis)

    struct {
        float min = -2.0f;
        float max = 2.0f;
    } zLimit; // Для объекта 3 (Z-axis)
};

std::vector<ObjectTransform> objectTransforms;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

glm::mat4 calculateModelMatrix(int index) {
    glm::mat4 model = glm::mat4(1.0f);

    switch (index) {
    case 0: // Базовый элемент
        break;

    case 1: // Вверх-вниз (Y-axis) + зависимость от объекта 2
        model = glm::translate(model, glm::vec3(objectTransforms[2].position.x, 0.0f, 0.0f));
        model = glm::translate(model, glm::vec3(0.0f, objectTransforms[1].position.y, 0.0f));
        break;

    case 2: // Влево-вправо (X-axis)
        model = glm::translate(model, glm::vec3(objectTransforms[2].position.x, 0.0f, 0.0f));
        break;

    case 3: // Вперед-назад (Z-axis)
        model = glm::translate(model, glm::vec3(0.0f, 0.0f, objectTransforms[3].position.z));
        break;
    }

    return model;
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "3D Model Transformations", NULL, NULL);
    if (window == NULL) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    Shader shader("vertex_sheder.glsl", "fragment_shader.glsl");
    Model ourModel("xlience.obj");

    objectTransforms.resize(4);
    objectTransforms[1].yLimit = { -0.24f, 0.24f };
    objectTransforms[2].xLimit = { -0.81f, 0.35f };
    objectTransforms[3].zLimit = { 0.0f, 0.97f };

    shader.use();
    shader.setVec3("light.position", glm::vec3(1.2f, 1.0f, 2.0f));
    shader.setVec3("light.ambient", glm::vec3(1.0f, 0.8f, 0.6f));
    shader.setVec3("light.diffuse", glm::vec3(1.0f, 0.8f, 0.6f));
    shader.setVec3("light.specular", glm::vec3(1.0f));
    shader.setVec3("material.ambient", glm::vec3(0.5f, 0.5f, 0.5f));
    shader.setVec3("material.diffuse", glm::vec3(1.0f, 1.0f, 0.0f));
    shader.setVec3("material.specular", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setFloat("material.shininess", 32.0f);

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader.use();
        shader.setVec3("viewPos", cameraPos);

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            (float)SCR_WIDTH / (float)SCR_HEIGHT,
            0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        shader.setMat4("projection", projection);
        shader.setMat4("view", view);

        for (size_t i = 0; i < ourModel.meshTransforms.size(); ++i) {
            ourModel.meshTransforms[i] = calculateModelMatrix(i);
        }

        ourModel.Draw(shader);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Управление камерой
    float cameraSpeed = 2.5f * deltaTime;
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        cameraPos += cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        cameraPos -= cameraSpeed * cameraFront;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

    // Управление объектами
    float moveSpeed = 1.5f * deltaTime;

    // Объект 1: Y-axis (Y/H)
    if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS) {
        objectTransforms[1].position.y += moveSpeed;
        objectTransforms[1].position.y = glm::clamp(
            objectTransforms[1].position.y,
            objectTransforms[1].yLimit.min,
            objectTransforms[1].yLimit.max
        );
    }
    if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
        objectTransforms[1].position.y -= moveSpeed;
        objectTransforms[1].position.y = glm::clamp(
            objectTransforms[1].position.y,
            objectTransforms[1].yLimit.min,
            objectTransforms[1].yLimit.max
        );
    }

    // Объект 2: X-axis (I/K)
    if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS) {
        objectTransforms[2].position.x -= moveSpeed;
        objectTransforms[2].position.x = glm::clamp(
            objectTransforms[2].position.x,
            objectTransforms[2].xLimit.min,
            objectTransforms[2].xLimit.max
        );
    }
    if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS) {
        objectTransforms[2].position.x += moveSpeed;
        objectTransforms[2].position.x = glm::clamp(
            objectTransforms[2].position.x,
            objectTransforms[2].xLimit.min,
            objectTransforms[2].xLimit.max
        );
    }

    // Объект 3: Z-axis (U/J)
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS) {
        objectTransforms[3].position.z -= moveSpeed;
        objectTransforms[3].position.z = glm::clamp(
            objectTransforms[3].position.z,
            objectTransforms[3].zLimit.min,
            objectTransforms[3].zLimit.max
        );
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS) {
        objectTransforms[3].position.z += moveSpeed;
        objectTransforms[3].position.z = glm::clamp(
            objectTransforms[3].position.z,
            objectTransforms[3].zLimit.min,
            objectTransforms[3].zLimit.max
        );
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
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
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
}
