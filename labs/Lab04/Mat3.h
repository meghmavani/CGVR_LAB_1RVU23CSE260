#pragma once

struct Vec2 {
    float x, y;
};

// Row-major 3x3 matrix for 2D homogeneous-coordinate transformations.
struct Mat3 {
    float m[3][3];
};

Mat3 identityMatrix();
Mat3 translationMatrix(float tx, float ty);
Mat3 rotationMatrix(float degrees);
Mat3 scalingMatrix(float sx, float sy);
Mat3 reflectionMatrixX();
Mat3 reflectionMatrixY();
Mat3 shearingMatrix(float shx, float shy);

Mat3 multiply(const Mat3& a, const Mat3& b);
Vec2 applyTransform(const Mat3& mat, Vec2 point);
