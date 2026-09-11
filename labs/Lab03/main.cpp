#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Mat3.h"

#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 700;

// An asymmetric right triangle (rather than a square) so that reflections
// and rotations produce a visibly different outline instead of mapping
// back onto themselves.
std::vector<Vec2> rightTriangle() {
    return {
        {-30, -25},
        {30, -25},
        {-30, 25},
    };
}

std::vector<float> toVertices(const std::vector<Vec2>& points, const Mat3& transform, Vec2 slotCenter) {
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2);

    for (const Vec2& point : points) {
        Vec2 transformed = applyTransform(transform, point);
        float worldX = transformed.x + slotCenter.x;
        float worldY = transformed.y + slotCenter.y;
        vertices.push_back(worldX / (WINDOW_WIDTH / 2.0f));
        vertices.push_back(worldY / (WINDOW_HEIGHT / 2.0f));
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab03 - 2D Transformations (Homogeneous Coordinates)", nullptr, nullptr);
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

    std::vector<Vec2> shape = rightTriangle();

    // Six slots arranged in a 3x2 grid: original, translation, rotation,
    // scaling, reflection, shearing. Each slot draws the untransformed
    // square dimly and the transformed square brightly, both centered on
    // the slot so the effect of the transform is easy to compare.
    struct Slot {
        Vec2 center;
        Mat3 transform;
        const char* label;
    };

    std::vector<Slot> slots = {
        {{-280, 190}, identityMatrix(), "Original"},
        {{0, 190}, translationMatrix(60, -30), "Translation"},
        {{280, 190}, rotationMatrix(45.0f), "Rotation"},
        {{-280, -190}, scalingMatrix(1.6f, 1.6f), "Scaling"},
        {{0, -190}, reflectionMatrixY(), "Reflection"},
        {{280, -190}, shearingMatrix(0.6f, 0.0f), "Shearing"},
    };

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

    std::vector<ShapeBuffer> originalBuffers;
    std::vector<ShapeBuffer> transformedBuffers;

    for (const Slot& slot : slots) {
        originalBuffers.push_back(createShapeBuffer(toVertices(shape, identityMatrix(), slot.center)));
        transformedBuffers.push_back(createShapeBuffer(toVertices(shape, slot.transform, slot.center)));
    }

    const GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        for (std::size_t i = 0; i < slots.size(); ++i) {
            glBindVertexArray(originalBuffers[i].vao);
            glUniform4f(colorLocation, 0.6f, 0.6f, 0.6f, 1.0f);
            glDrawArrays(GL_LINE_LOOP, 0, originalBuffers[i].vertexCount);

            glBindVertexArray(transformedBuffers[i].vao);
            glUniform4f(colorLocation, 0.2f, 0.9f, 0.5f, 1.0f);
            glDrawArrays(GL_LINE_LOOP, 0, transformedBuffers[i].vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    for (std::size_t i = 0; i < slots.size(); ++i) {
        glDeleteVertexArrays(1, &originalBuffers[i].vao);
        glDeleteBuffers(1, &originalBuffers[i].vbo);
        glDeleteVertexArrays(1, &transformedBuffers[i].vao);
        glDeleteBuffers(1, &transformedBuffers[i].vbo);
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
