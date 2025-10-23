#pragma once

#include "Vector3.h"

class Matrix4 {
public:
    float m[16];
    
    Matrix4();
    Matrix4(const Matrix4& other);
    
    // Assignment
    Matrix4& operator=(const Matrix4& other);
    
    // Matrix operations
    Matrix4 operator*(const Matrix4& other) const;
    Vector3 operator*(const Vector3& vector) const;
    
    // Accessors
    float& operator()(int row, int col);
    const float& operator()(int row, int col) const;
    
    // Static constructors
    static Matrix4 identity();
    static Matrix4 translation(const Vector3& translation);
    static Matrix4 rotation(const Vector3& axis, float angle);
    static Matrix4 rotationX(float angle);
    static Matrix4 rotationY(float angle);
    static Matrix4 rotationZ(float angle);
    static Matrix4 scale(const Vector3& scale);
    static Matrix4 scale(float uniformScale);
    static Matrix4 perspective(float fov, float aspect, float near, float far);
    static Matrix4 orthographic(float left, float right, float bottom, float top, float near, float far);
    static Matrix4 lookAt(const Vector3& eye, const Vector3& center, const Vector3& up);
    
    // Transformations
    void translate(const Vector3& translation);
    void rotate(const Vector3& axis, float angle);
    void scale(const Vector3& scale);
    
    // Utilities
    Matrix4 transposed() const;
    Matrix4 inverted() const;
    float determinant() const;
    
    // Vector transformations
    Vector3 transformPoint(const Vector3& point) const;
    Vector3 transformDirection(const Vector3& direction) const;
    Vector3 transformNormal(const Vector3& normal) const;
};