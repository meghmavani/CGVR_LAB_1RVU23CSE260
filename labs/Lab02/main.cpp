#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "MidpointCircle.h"
#include "Point.h"

#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::vector<float> toVertices(const std::vector<Point>& points) {
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2);

    for (const Point& point : points) {
        float ndcX = static_cast<float>(point.x) / (WINDOW_WIDTH / 2.0f);
        float ndcY = static_cast<float>(point.y) / (WINDOW_HEIGHT / 2.0f);
        vertices.push_back(ndcX);
        vertices.push_back(ndcY);
    }

    return vertices;
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab02 - Midpoint Circle Algorithm", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    if (!gladLoadGL(glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    int framebufferWidth, framebufferHeight;
    glfwGetFramebufferSize(window, &framebufferWidth, &framebufferHeight);
    glViewport(0, 0, framebufferWidth, framebufferHeight);

    // Three concentric circles at different radii to show the algorithm is
    // radius-independent and symmetric across all eight octants.
    std::vector<Point> circleSmall = midpointCircle(0, 0, 80);
    std::vector<Point> circleMedium = midpointCircle(0, 0, 140);
    std::vector<Point> circleLarge = midpointCircle(0, 0, 200);

    std::vector<float> smallVertices = toVertices(circleSmall);
    std::vector<float> mediumVertices = toVertices(circleMedium);
    std::vector<float> largeVertices = toVertices(circleLarge);

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
            gl_PointSize = 2.0;
        }
    )";

    const char* fragmentShaderSrc = R"(
        #version 330 core
        uniform vec4 uColor;
        out vec4 FragColor;
        void main() {
            FragColor = uColor;
        }
    )";

    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSrc, nullptr);
    glCompileShader(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSrc, nullptr);
    glCompileShader(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    auto createPointBuffer = [](const std::vector<float>& vertices, GLuint& vao, GLuint& vbo) {
        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
    };

    GLuint smallVAO, smallVBO;
    GLuint mediumVAO, mediumVBO;
    GLuint largeVAO, largeVBO;

    createPointBuffer(smallVertices, smallVAO, smallVBO);
    createPointBuffer(mediumVertices, mediumVAO, mediumVBO);
    createPointBuffer(largeVertices, largeVAO, largeVBO);

    glEnable(GL_PROGRAM_POINT_SIZE);

    const GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(smallVAO);
        glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(circleSmall.size()));

        glBindVertexArray(mediumVAO);
        glUniform4f(colorLocation, 0.2f, 0.8f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(circleMedium.size()));

        glBindVertexArray(largeVAO);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(circleLarge.size()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &smallVAO);
    glDeleteBuffers(1, &smallVBO);
    glDeleteVertexArrays(1, &mediumVAO);
    glDeleteBuffers(1, &mediumVBO);
    glDeleteVertexArrays(1, &largeVAO);
    glDeleteBuffers(1, &largeVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
