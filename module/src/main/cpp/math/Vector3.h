#pragma once

#include <cmath>

class Vector3 {
public:
    float x, y, z;
    
    Vector3();
    Vector3(float x, float y, float z);
    Vector3(const Vector3& other);
    
    // Assignment
    Vector3& operator=(const Vector3& other);
    
    // Arithmetic operations
    Vector3 operator+(const Vector3& other) const;
    Vector3 operator-(const Vector3& other) const;
    Vector3 operator*(float scalar) const;
    Vector3 operator/(float scalar) const;
    
    Vector3& operator+=(const Vector3& other);
    Vector3& operator-=(const Vector3& other);
    Vector3& operator*=(float scalar);
    Vector3& operator/=(float scalar);
    
    // Comparison
    bool operator==(const Vector3& other) const;
    bool operator!=(const Vector3& other) const;
    
    // Vector operations
    float length() const;
    float lengthSquared() const;
    Vector3 normalized() const;
    void normalize();
    
    float dot(const Vector3& other) const;
    Vector3 cross(const Vector3& other) const;
    
    // Distance
    float distance(const Vector3& other) const;
    float distanceSquared(const Vector3& other) const;
    
    // Static methods
    static Vector3 zero();
    static Vector3 one();
    static Vector3 up();
    static Vector3 down();
    static Vector3 left();
    static Vector3 right();
    static Vector3 forward();
    static Vector3 back();
    
    // Lerp
    static Vector3 lerp(const Vector3& a, const Vector3& b, float t);
};