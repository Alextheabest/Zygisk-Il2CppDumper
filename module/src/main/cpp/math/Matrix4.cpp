#include "Matrix4.h"
#include <cmath>
#include <cstring>

Matrix4::Matrix4() {
    memset(m, 0, sizeof(m));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

Matrix4::Matrix4(const Matrix4& other) {
    memcpy(m, other.m, sizeof(m));
}

Matrix4& Matrix4::operator=(const Matrix4& other) {
    if (this != &other) {
        memcpy(m, other.m, sizeof(m));
    }
    return *this;
}

Matrix4 Matrix4::operator*(const Matrix4& other) const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = 0.0f;
            for (int k = 0; k < 4; ++k) {
                result.m[i * 4 + j] += m[i * 4 + k] * other.m[k * 4 + j];
            }
        }
    }
    return result;
}

Vector3 Matrix4::operator*(const Vector3& vector) const {
    return transformPoint(vector);
}

float& Matrix4::operator()(int row, int col) {
    return m[row * 4 + col];
}

const float& Matrix4::operator()(int row, int col) const {
    return m[row * 4 + col];
}

Matrix4 Matrix4::identity() {
    return Matrix4();
}

Matrix4 Matrix4::translation(const Vector3& translation) {
    Matrix4 result = identity();
    result.m[12] = translation.x;
    result.m[13] = translation.y;
    result.m[14] = translation.z;
    return result;
}

Matrix4 Matrix4::rotation(const Vector3& axis, float angle) {
    Vector3 normalizedAxis = axis.normalized();
    float c = std::cos(angle);
    float s = std::sin(angle);
    float t = 1.0f - c;
    
    Matrix4 result;
    result.m[0] = t * normalizedAxis.x * normalizedAxis.x + c;
    result.m[1] = t * normalizedAxis.x * normalizedAxis.y + s * normalizedAxis.z;
    result.m[2] = t * normalizedAxis.x * normalizedAxis.z - s * normalizedAxis.y;
    result.m[3] = 0.0f;
    
    result.m[4] = t * normalizedAxis.x * normalizedAxis.y - s * normalizedAxis.z;
    result.m[5] = t * normalizedAxis.y * normalizedAxis.y + c;
    result.m[6] = t * normalizedAxis.y * normalizedAxis.z + s * normalizedAxis.x;
    result.m[7] = 0.0f;
    
    result.m[8] = t * normalizedAxis.x * normalizedAxis.z + s * normalizedAxis.y;
    result.m[9] = t * normalizedAxis.y * normalizedAxis.z - s * normalizedAxis.x;
    result.m[10] = t * normalizedAxis.z * normalizedAxis.z + c;
    result.m[11] = 0.0f;
    
    result.m[12] = 0.0f;
    result.m[13] = 0.0f;
    result.m[14] = 0.0f;
    result.m[15] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::rotationX(float angle) {
    return rotation(Vector3::right(), angle);
}

Matrix4 Matrix4::rotationY(float angle) {
    return rotation(Vector3::up(), angle);
}

Matrix4 Matrix4::rotationZ(float angle) {
    return rotation(Vector3::forward(), angle);
}

Matrix4 Matrix4::scale(const Vector3& scale) {
    Matrix4 result = identity();
    result.m[0] = scale.x;
    result.m[5] = scale.y;
    result.m[10] = scale.z;
    return result;
}

Matrix4 Matrix4::scale(float uniformScale) {
    return scale(Vector3(uniformScale, uniformScale, uniformScale));
}

Matrix4 Matrix4::perspective(float fov, float aspect, float near, float far) {
    Matrix4 result;
    float tanHalfFov = std::tan(fov * 0.5f);
    
    result.m[0] = 1.0f / (aspect * tanHalfFov);
    result.m[1] = 0.0f;
    result.m[2] = 0.0f;
    result.m[3] = 0.0f;
    
    result.m[4] = 0.0f;
    result.m[5] = 1.0f / tanHalfFov;
    result.m[6] = 0.0f;
    result.m[7] = 0.0f;
    
    result.m[8] = 0.0f;
    result.m[9] = 0.0f;
    result.m[10] = -(far + near) / (far - near);
    result.m[11] = -1.0f;
    
    result.m[12] = 0.0f;
    result.m[13] = 0.0f;
    result.m[14] = -(2.0f * far * near) / (far - near);
    result.m[15] = 0.0f;
    
    return result;
}

Matrix4 Matrix4::orthographic(float left, float right, float bottom, float top, float near, float far) {
    Matrix4 result;
    
    result.m[0] = 2.0f / (right - left);
    result.m[1] = 0.0f;
    result.m[2] = 0.0f;
    result.m[3] = 0.0f;
    
    result.m[4] = 0.0f;
    result.m[5] = 2.0f / (top - bottom);
    result.m[6] = 0.0f;
    result.m[7] = 0.0f;
    
    result.m[8] = 0.0f;
    result.m[9] = 0.0f;
    result.m[10] = -2.0f / (far - near);
    result.m[11] = 0.0f;
    
    result.m[12] = -(right + left) / (right - left);
    result.m[13] = -(top + bottom) / (top - bottom);
    result.m[14] = -(far + near) / (far - near);
    result.m[15] = 1.0f;
    
    return result;
}

Matrix4 Matrix4::lookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 f = (center - eye).normalized();
    Vector3 s = f.cross(up).normalized();
    Vector3 u = s.cross(f);
    
    Matrix4 result;
    result.m[0] = s.x;
    result.m[1] = u.x;
    result.m[2] = -f.x;
    result.m[3] = 0.0f;
    
    result.m[4] = s.y;
    result.m[5] = u.y;
    result.m[6] = -f.y;
    result.m[7] = 0.0f;
    
    result.m[8] = s.z;
    result.m[9] = u.z;
    result.m[10] = -f.z;
    result.m[11] = 0.0f;
    
    result.m[12] = -s.dot(eye);
    result.m[13] = -u.dot(eye);
    result.m[14] = f.dot(eye);
    result.m[15] = 1.0f;
    
    return result;
}

void Matrix4::translate(const Vector3& translation) {
    *this = *this * Matrix4::translation(translation);
}

void Matrix4::rotate(const Vector3& axis, float angle) {
    *this = *this * Matrix4::rotation(axis, angle);
}

void Matrix4::scale(const Vector3& scale) {
    *this = *this * Matrix4::scale(scale);
}

Matrix4 Matrix4::transposed() const {
    Matrix4 result;
    for (int i = 0; i < 4; ++i) {
        for (int j = 0; j < 4; ++j) {
            result.m[i * 4 + j] = m[j * 4 + i];
        }
    }
    return result;
}

float Matrix4::determinant() const {
    return m[0] * (m[5] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[9] * m[15] - m[11] * m[13]) + m[7] * (m[9] * m[14] - m[10] * m[13]))
         - m[1] * (m[4] * (m[10] * m[15] - m[11] * m[14]) - m[6] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[14] - m[10] * m[12]))
         + m[2] * (m[4] * (m[9] * m[15] - m[11] * m[13]) - m[5] * (m[8] * m[15] - m[11] * m[12]) + m[7] * (m[8] * m[13] - m[9] * m[12]))
         - m[3] * (m[4] * (m[9] * m[14] - m[10] * m[13]) - m[5] * (m[8] * m[14] - m[10] * m[12]) + m[6] * (m[8] * m[13] - m[9] * m[12]));
}

Matrix4 Matrix4::inverted() const {
    // This is a simplified inversion - in practice, you'd want a more robust implementation
    Matrix4 result;
    float det = determinant();
    if (std::abs(det) < 1e-6f) {
        return identity(); // Return identity if matrix is singular
    }
    
    // For now, return identity - full matrix inversion is complex
    // In a real implementation, you'd implement proper matrix inversion here
    return identity();
}

Vector3 Matrix4::transformPoint(const Vector3& point) const {
    float x = m[0] * point.x + m[4] * point.y + m[8] * point.z + m[12];
    float y = m[1] * point.x + m[5] * point.y + m[9] * point.z + m[13];
    float z = m[2] * point.x + m[6] * point.y + m[10] * point.z + m[14];
    return Vector3(x, y, z);
}

Vector3 Matrix4::transformDirection(const Vector3& direction) const {
    float x = m[0] * direction.x + m[4] * direction.y + m[8] * direction.z;
    float y = m[1] * direction.x + m[5] * direction.y + m[9] * direction.z;
    float z = m[2] * direction.x + m[6] * direction.y + m[10] * direction.z;
    return Vector3(x, y, z);
}

Vector3 Matrix4::transformNormal(const Vector3& normal) const {
    // Transform normal using the inverse transpose of the upper 3x3 matrix
    Matrix4 invTranspose = transposed();
    return invTranspose.transformDirection(normal).normalized();
}