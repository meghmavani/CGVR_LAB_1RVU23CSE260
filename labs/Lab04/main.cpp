#include <glad/gl.h>
#include <GLFW/glfw3.h>

#include "Mat3.h"

#include <cmath>
#include <iostream>
#include <vector>

const int WINDOW_WIDTH = 900;
const int WINDOW_HEIGHT = 700;

// An asymmetric right triangle so rotation/scale/translate composites are
// all visually distinguishable from one another.
std::vector<Vec2> rightTriangle() {
    return {
        {-25, -20},
        {25, -20},
        {-25, 20},
    };
}

std::vector<float> toVertices(const std::vector<Vec2>& points, const Mat3& transform) {
    std::vector<float> vertices;
    vertices.reserve(points.size() * 2);

    for (const Vec2& point : points) {
        Vec2 transformed = applyTransform(transform, point);
        vertices.push_back(transformed.x / (WINDOW_WIDTH / 2.0f));
        vertices.push_back(transformed.y / (WINDOW_HEIGHT / 2.0f));
    }

    return vertices;
}

// Verifies that applying one precomputed composite matrix M = T * R * S to a
// point gives the same result as applying S, then R, then T sequentially to
// that point. This is the core idea behind composite transformations: chain
// the matrices first, then transform geometry with a single multiply.
void verifyCompositeMatchesSequential(const Mat3& translate, const Mat3& rotate, const Mat3& scale, Vec2 samplePoint) {
    Mat3 composite = multiply(translate, multiply(rotate, scale));
    Vec2 viaComposite = applyTransform(composite, samplePoint);

    Vec2 viaSequential = applyTransform(scale, samplePoint);
    viaSequential = applyTransform(rotate, viaSequential);
    viaSequential = applyTransform(translate, viaSequential);

    float dx = std::abs(viaComposite.x - viaSequential.x);
    float dy = std::abs(viaComposite.y - viaSequential.y);
    const float epsilon = 0.001f;

    std::cout << "Composite matrix result:   (" << viaComposite.x << ", " << viaComposite.y << ")\n";
    std::cout << "Sequential transform result: (" << viaSequential.x << ", " << viaSequential.y << ")\n";
    std::cout << ((dx < epsilon && dy < epsilon) ? "PASS: composite matrix matches sequential transforms\n"
                                                  : "FAIL: composite matrix diverges from sequential transforms\n");
    std::cout.flush();
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

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Lab04 - Composite Transformations", nullptr, nullptr);
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

    struct Composite {
        Mat3 translate;
        Mat3 rotate;
        Mat3 scale;
    };

    // Four different composite transforms: each combines translation,
    // rotation and scaling into a single matrix (M = T * R * S) applied to
    // the shape in one multiply per vertex.
    std::vector<Composite> composites = {
        {translationMatrix(220, 150), rotationMatrix(30.0f), scalingMatrix(1.6f, 1.6f)},
        {translationMatrix(-220, 150), rotationMatrix(-45.0f), scalingMatrix(1.0f, 1.0f)},
        {translationMatrix(220, -150), rotationMatrix(60.0f), scalingMatrix(1.3f, 2.0f)},
        {translationMatrix(-220, -150), rotationMatrix(120.0f), scalingMatrix(1.2f, 1.2f)},
    };

    verifyCompositeMatchesSequential(composites[0].translate, composites[0].rotate, composites[0].scale, Vec2{10.0f, 15.0f});

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

    ShapeBuffer originalBuffer = createShapeBuffer(toVertices(shape, identityMatrix()));

    std::vector<ShapeBuffer> compositeBuffers;
    for (const Composite& c : composites) {
        Mat3 composite = multiply(c.translate, multiply(c.rotate, c.scale));
        compositeBuffers.push_back(createShapeBuffer(toVertices(shape, composite)));
    }

    const GLint colorLocation = glGetUniformLocation(shaderProgram, "uColor");

    const float colors[4][3] = {
        {0.2f, 0.9f, 0.5f},
        {0.2f, 0.8f, 1.0f},
        {1.0f, 0.8f, 0.2f},
        {1.0f, 0.4f, 0.6f},
    };

    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, true);
        }

        glClearColor(0.05f, 0.05f, 0.05f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(shaderProgram);

        glBindVertexArray(originalBuffer.vao);
        glUniform4f(colorLocation, 0.6f, 0.6f, 0.6f, 1.0f);
        glDrawArrays(GL_LINE_LOOP, 0, originalBuffer.vertexCount);

        for (std::size_t i = 0; i < compositeBuffers.size(); ++i) {
            glBindVertexArray(compositeBuffers[i].vao);
            glUniform4f(colorLocation, colors[i][0], colors[i][1], colors[i][2], 1.0f);
            glDrawArrays(GL_LINE_LOOP, 0, compositeBuffers[i].vertexCount);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &originalBuffer.vao);
    glDeleteBuffers(1, &originalBuffer.vbo);
    for (ShapeBuffer& buffer : compositeBuffers) {
        glDeleteVertexArrays(1, &buffer.vao);
        glDeleteBuffers(1, &buffer.vbo);
    }
    glDeleteProgram(shaderProgram);

    glfwTerminate();
    return 0;
}
