#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "DDA.h"
#include "Point.h"

#include <cstdlib>
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::vector<Point> offsetPoints(const std::vector<Point>& points, int dx, int dy) {
    std::vector<Point> shifted;
    shifted.reserve(points.size());

    for (const Point& point : points) {
        shifted.push_back({point.x + dx, point.y + dy});
    }

    return shifted;
}

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

std::vector<Point> bresenhamLine(int x0, int y0, int x1, int y1) {
    std::vector<Point> points;

    int dx = std::abs(x1 - x0);
    int dy = -std::abs(y1 - y0);
    int sx = (x0 < x1) ? 1 : -1;
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        points.push_back({x0, y0});
        if (x0 == x1 && y0 == y1) break;

        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }

    return points;
}

// Midpoint circle drawing algorithm
std::vector<Point> midpointCircle(int cx, int cy, int radius) {
    std::vector<Point> points;

    int x = radius;
    int y = 0;
    int err = 1 - radius;

    auto plotOctants = [&](int x, int y) {
        points.push_back({cx + x, cy + y});
        points.push_back({cx + y, cy + x});
        points.push_back({cx - y, cy + x});
        points.push_back({cx - x, cy + y});
        points.push_back({cx - x, cy - y});
        points.push_back({cx - y, cy - x});
        points.push_back({cx + y, cy - x});
        points.push_back({cx + x, cy - y});
    };

    while (x >= y) {
        plotOctants(x, y);
        y++;
        if (err < 0) {
            err += 2 * y + 1;
        } else {
            x--;
            err += 2 * (y - x) + 1;
        }
    }

    return points;
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab01 - Bresenham vs DDA", nullptr, nullptr);
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

    // Compare Bresenham and DDA side by side, keep the circle below.
    std::vector<Point> line1 = offsetPoints(bresenhamLine(-120, -80, 120, 80), -220, 120);
    std::vector<Point> line2 = offsetPoints(ddaLine(-120, 80, 120, -80), 220, 120);
    std::vector<Point> circle = offsetPoints(midpointCircle(0, 0, 100), 0, -130);

    std::vector<float> line1Vertices = toVertices(line1);
    std::vector<float> line2Vertices = toVertices(line2);
    std::vector<float> circleVertices = toVertices(circle);

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

    GLuint line1VAO, line1VBO;
    GLuint line2VAO, line2VBO;
    GLuint circleVAO, circleVBO;

    createPointBuffer(line1Vertices, line1VAO, line1VBO);
    createPointBuffer(line2Vertices, line2VAO, line2VBO);
    createPointBuffer(circleVertices, circleVAO, circleVBO);

    glEnable(GL_PROGRAM_POINT_SIZE);

    const GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(line1VAO);
        glUniform4f(colorLocation, 1.0f, 0.3f, 0.3f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(line1.size()));

        glBindVertexArray(line2VAO);
        glUniform4f(colorLocation, 0.2f, 0.8f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(line2.size()));

        glBindVertexArray(circleVAO);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_POINTS, 0, static_cast<GLsizei>(circle.size()));

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &line1VAO);
    glDeleteBuffers(1, &line1VBO);
    glDeleteVertexArrays(1, &line2VAO);
    glDeleteBuffers(1, &line2VBO);
    glDeleteVertexArrays(1, &circleVAO);
    glDeleteBuffers(1, &circleVBO);
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
