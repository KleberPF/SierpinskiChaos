#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <glm/glm.hpp>

// configurations
constexpr unsigned int SCR_WIDTH = 1600;
constexpr unsigned int SCR_HEIGHT = 900;

constexpr int numberOfPoints = 5000;
constexpr float interval = 5; // time for each point to spawn in ms
constexpr glm::vec3 backgroundColor(0.0f, 0.0f, 0.0f);
constexpr glm::vec3 pointColor(0.14f, 0.65f, 0.047f);

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

void initGLFW();
GLFWwindow* createWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share);
bool initGLAD();
unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath);

const char* vertexShaderSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;
void main()
{
    gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0);
})";

const char* fragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;

void main()
{
    FragColor = vec4(color, 1.0f);
})";

int main()
{
    initGLFW();
    GLFWwindow* window = createWindow(SCR_WIDTH, SCR_HEIGHT, "Sierpinski from Chaos", glfwGetPrimaryMonitor(), NULL);

    if (!initGLAD())
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    unsigned int shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);
    glUseProgram(shaderProgram);

    // set point color
    glUniform3fv(glGetUniformLocation(shaderProgram, "color"), 1, &pointColor[0]);

    // calculate the points
    // start vector with 3 initial coordinates
    std::vector<glm::vec2> vertices
    {
        glm::vec2(-0.5f, -0.5f), // left  
        glm::vec2( 0.5f, -0.5f), // right 
        glm::vec2( 0.0f,  0.5f), // top
        glm::vec2( 0.2f, -0.2f)  // random point
    };
    
    glm::vec2 currentPoint = vertices[3];
    
    std::srand(std::time(nullptr));
    for (int i = 0; i < numberOfPoints - 4; ++i)
    {
        int roll = (rand() % 3);
        glm::vec2 newPoint = (currentPoint + vertices[roll]) / 2.0f;
        vertices.push_back(newPoint);
        currentPoint = newPoint;
    }

    unsigned int VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec2), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // control the rendering
    float currentTime = interval;
    float pointsCount = 4;

    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(backgroundColor.x , backgroundColor.y, backgroundColor.z, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glPointSize(2.0f);

        if (glfwGetTime() * 1000 > currentTime && pointsCount < numberOfPoints)
        {
            ++pointsCount;
            currentTime += interval;
        }

        glUseProgram(shaderProgram);
        glBindVertexArray(VAO);
        glDrawArrays(GL_POINTS, 0, pointsCount);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

void initGLFW()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
}

GLFWwindow* createWindow(int width, int height, const char* title, GLFWmonitor* monitor, GLFWwindow* share)
{
    GLFWwindow* window = glfwCreateWindow(width, height, title, monitor, share);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    return window;
}

bool initGLAD()
{
    return (bool)gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
}

unsigned int createShaderProgram(const char* vertexPath, const char* fragmentPath)
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
}

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    }

    unsigned int shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return shaderProgram;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}