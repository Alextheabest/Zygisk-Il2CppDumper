#include "Vector3.h"

Vector3::Vector3() : x(0.0f), y(0.0f), z(0.0f) {}

Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}

Vector3::Vector3(const Vector3& other) : x(other.x), y(other.y), z(other.z) {}

Vector3& Vector3::operator=(const Vector3& other) {
    if (this != &other) {
        x = other.x;
        y = other.y;
        z = other.z;
    }
    return *this;
}

Vector3 Vector3::operator+(const Vector3& other) const {
    return Vector3(x + other.x, y + other.y, z + other.z);
}

Vector3 Vector3::operator-(const Vector3& other) const {
    return Vector3(x - other.x, y - other.y, z - other.z);
}

Vector3 Vector3::operator*(float scalar) const {
    return Vector3(x * scalar, y * scalar, z * scalar);
}

Vector3 Vector3::operator/(float scalar) const {
    return Vector3(x / scalar, y / scalar, z / scalar);
}

Vector3& Vector3::operator+=(const Vector3& other) {
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}

Vector3& Vector3::operator-=(const Vector3& other) {
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

Vector3& Vector3::operator*=(float scalar) {
    x *= scalar;
    y *= scalar;
    z *= scalar;
    return *this;
}

Vector3& Vector3::operator/=(float scalar) {
    x /= scalar;
    y /= scalar;
    z /= scalar;
    return *this;
}

bool Vector3::operator==(const Vector3& other) const {
    return x == other.x && y == other.y && z == other.z;
}

bool Vector3::operator!=(const Vector3& other) const {
    return !(*this == other);
}

float Vector3::length() const {
    return std::sqrt(x * x + y * y + z * z);
}

float Vector3::lengthSquared() const {
    return x * x + y * y + z * z;
}

Vector3 Vector3::normalized() const {
    float len = length();
    if (len > 0.0f) {
        return *this / len;
    }
    return Vector3::zero();
}

void Vector3::normalize() {
    float len = length();
    if (len > 0.0f) {
        *this /= len;
    }
}

float Vector3::dot(const Vector3& other) const {
    return x * other.x + y * other.y + z * other.z;
}

Vector3 Vector3::cross(const Vector3& other) const {
    return Vector3(
        y * other.z - z * other.y,
        z * other.x - x * other.z,
        x * other.y - y * other.x
    );
}

float Vector3::distance(const Vector3& other) const {
    return (*this - other).length();
}

float Vector3::distanceSquared(const Vector3& other) const {
    return (*this - other).lengthSquared();
}

Vector3 Vector3::zero() {
    return Vector3(0.0f, 0.0f, 0.0f);
}

Vector3 Vector3::one() {
    return Vector3(1.0f, 1.0f, 1.0f);
}

Vector3 Vector3::up() {
    return Vector3(0.0f, 1.0f, 0.0f);
}

Vector3 Vector3::down() {
    return Vector3(0.0f, -1.0f, 0.0f);
}

Vector3 Vector3::left() {
    return Vector3(-1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::right() {
    return Vector3(1.0f, 0.0f, 0.0f);
}

Vector3 Vector3::forward() {
    return Vector3(0.0f, 0.0f, -1.0f);
}

Vector3 Vector3::back() {
    return Vector3(0.0f, 0.0f, 1.0f);
}

Vector3 Vector3::lerp(const Vector3& a, const Vector3& b, float t) {
    return a + (b - a) * t;
}