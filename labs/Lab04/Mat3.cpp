#include "Mat3.h"

#include <cmath>

Mat3 identityMatrix() {
    return Mat3{{
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
    }};
}

Mat3 translationMatrix(float tx, float ty) {
    Mat3 mat = identityMatrix();
    mat.m[0][2] = tx;
    mat.m[1][2] = ty;
    return mat;
}

Mat3 rotationMatrix(float degrees) {
    float radians = degrees * static_cast<float>(M_PI) / 180.0f;
    float c = std::cos(radians);
    float s = std::sin(radians);

    Mat3 mat = identityMatrix();
    mat.m[0][0] = c;
    mat.m[0][1] = -s;
    mat.m[1][0] = s;
    mat.m[1][1] = c;
    return mat;
}

Mat3 scalingMatrix(float sx, float sy) {
    Mat3 mat = identityMatrix();
    mat.m[0][0] = sx;
    mat.m[1][1] = sy;
    return mat;
}

Mat3 reflectionMatrixX() {
    // Reflects across the x-axis (flips y).
    return scalingMatrix(1.0f, -1.0f);
}

Mat3 reflectionMatrixY() {
    // Reflects across the y-axis (flips x).
    return scalingMatrix(-1.0f, 1.0f);
}

Mat3 shearingMatrix(float shx, float shy) {
    Mat3 mat = identityMatrix();
    mat.m[0][1] = shx;
    mat.m[1][0] = shy;
    return mat;
}

Mat3 multiply(const Mat3& a, const Mat3& b) {
    Mat3 result{};
    for (int row = 0; row < 3; ++row) {
        for (int col = 0; col < 3; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 3; ++k) {
                sum += a.m[row][k] * b.m[k][col];
            }
            result.m[row][col] = sum;
        }
    }
    return result;
}

Vec2 applyTransform(const Mat3& mat, Vec2 point) {
    float x = mat.m[0][0] * point.x + mat.m[0][1] * point.y + mat.m[0][2];
    float y = mat.m[1][0] * point.x + mat.m[1][1] * point.y + mat.m[1][2];
    return Vec2{x, y};
}
