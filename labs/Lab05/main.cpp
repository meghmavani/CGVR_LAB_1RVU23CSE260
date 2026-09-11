#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "CohenSutherland.h"

#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 800;
const int WINDOW_HEIGHT = 600;

std::vector<float> toVertices(const std::vector<Vec2>& points) {
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2);

    for (const Vec2& point : points) {
        vertices.push_back(point.x / (WINDOW_WIDTH / 2.0f));
        vertices.push_back(point.y / (WINDOW_HEIGHT / 2.0f));
    }

    return vertices;
}

static void framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

struct ShapeBuffer {
    GLuint vao, vbo;
    GLsizei vertexCount;
};

ShapeBuffer createShapeBuffer(const std::vector<float>& vertices) {
    ShapeBuffer buffer{};
    buffer.vertexCount = static_cast<GLsizei>(vertices.size() / 2);

    glGenVertexArrays(1, &buffer.vao);
    glGenBuffers(1, &buffer.vbo);

    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    return buffer;
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab05 - Cohen-Sutherland Line Clipping", nullptr, nullptr);
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

    ClipWindow clipWindow{-150, -100, 150, 100};

    std::vector<Vec2> clipRectVertices = {
        {clipWindow.xmin, clipWindow.ymin},
        {clipWindow.xmax, clipWindow.ymin},
        {clipWindow.xmax, clipWindow.ymax},
        {clipWindow.xmin, clipWindow.ymax},
    };

    // Test lines chosen to exercise every Cohen-Sutherland case: fully
    // inside, fully outside (trivial reject), and crossing each of the
    // four edges plus a corner-cutting diagonal.
    struct TestLine {
        Vec2 p0, p1;
        const char* description;
    };

    std::vector<TestLine> testLines = {
        {{-80, -40}, {80, 40}, "fully inside"},
        {{200, -50}, {300, 50}, "fully outside (trivial reject)"},
        {{50, 0}, {250, 0}, "crosses right edge"},
        {{-250, 0}, {-50, 0}, "crosses left edge"},
        {{0, -50}, {0, 200}, "crosses top edge"},
        {{0, 50}, {0, -200}, "crosses bottom edge"},
        {{-250, -200}, {250, 200}, "diagonal crossing two edges"},
    };

    std::cout << "Cohen-Sutherland clip results (window: [" << clipWindow.xmin << ", " << clipWindow.xmax
              << "] x [" << clipWindow.ymin << ", " << clipWindow.ymax << "]):\n";

    std::vector<ClipResult> clipResults;
    for (const TestLine& line : testLines) {
        ClipResult result = cohenSutherlandClip(line.p0, line.p1, clipWindow);
        clipResults.push_back(result);

        std::cout << "  " << line.description << ": ";
        if (result.visible) {
            std::cout << "visible from (" << result.p0.x << ", " << result.p0.y << ") to (" << result.p1.x << ", "
                       << result.p1.y << ")\n";
        } else {
            std::cout << "clipped away entirely\n";
        }
    }
    std::cout.flush();

    const char* vertexShaderSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
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

    ShapeBuffer clipRectBuffer = createShapeBuffer(toVertices(clipRectVertices));

    std::vector<ShapeBuffer> originalBuffers;
    std::vector<ShapeBuffer> clippedBuffers;

    for (std::size_t i = 0; i < testLines.size(); ++i) {
        originalBuffers.push_back(createShapeBuffer(toVertices({testLines[i].p0, testLines[i].p1})));
        if (clipResults[i].visible) {
            clippedBuffers.push_back(createShapeBuffer(toVertices({clipResults[i].p0, clipResults[i].p1})));
        } else {
            clippedBuffers.push_back(ShapeBuffer{0, 0, 0});
        }
    }

    const GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");

    glLineWidth(2.0f);

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(clipRectBuffer.vao);
        glUniform4f(colorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, clipRectBuffer.vertexCount);

        for (std::size_t i = 0; i < testLines.size(); ++i) {
            glBindVertexArray(originalBuffers[i].vao);
            glUniform4f(colorLocation, 0.5f, 0.5f, 0.5f, 1.0f);
            glDrawArrays(GL_LINES, 0, originalBuffers[i].vertexCount);

            if (clippedBuffers[i].vertexCount > 0) {
                glBindVertexArray(clippedBuffers[i].vao);
                glUniform4f(colorLocation, 0.2f, 0.9f, 0.5f, 1.0f);
                glDrawArrays(GL_LINES, 0, clippedBuffers[i].vertexCount);
            }
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &clipRectBuffer.vao);
    glDeleteBuffers(1, &clipRectBuffer.vbo);
    for (ShapeBuffer& buffer : originalBuffers) {
        glDeleteVertexArrays(1, &buffer.vao);
        glDeleteBuffers(1, &buffer.vbo);
    }
    for (ShapeBuffer& buffer : clippedBuffers) {
        if (buffer.vertexCount > 0) {
            glDeleteVertexArrays(1, &buffer.vao);
            glDeleteBuffers(1, &buffer.vbo);
        }
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
