/*--- Compatible with -std=c++17 ---*/
#pragma once
#include <iostream>
#include <type_traits>

#if defined(__NVCC__)
#define ATTRIBUTE __device__ __host__
#else
#define ATTRIBUTE
#endif

template <typename T, typename dtype = double>
struct vec3 {
  using itype = int;

  union {
    struct {
      T x, y, z;
    };

    T _array[3];
  };

  ATTRIBUTE vec3() : x(0), y(0), z(0) {};
  ATTRIBUTE vec3(T x_, T y_, T z_) : x(x_), y(y_), z(z_) {};
  ATTRIBUTE vec3(T x_) : x(x_), y(x_), z(x_) {};
  ATTRIBUTE vec3(const vec3& a) : x(a.x), y(a.y), z(a.z) {};
  ATTRIBUTE ~vec3(){};

  ATTRIBUTE void set(T x_) {
    x = x_;
    y = x_;
    z = x_;
  };

  ATTRIBUTE void set(T x_, T y_, T z_) {
    x = x_;
    y = y_;
    z = z_;
  };

  // nvcc向けのエラー回避部分はそのまま残しています
  #if 0
  template <typename U, std::enable_if_t<std::is_integral_v<U>, int> = 0>
  ATTRIBUTE T& operator[](U i) { return _array[i]; }

  template <typename U, std::enable_if_t<std::is_integral_v<U>, int> = 0>
  ATTRIBUTE const T& operator[](U i) const { return _array[i]; }
  #else
  ATTRIBUTE T& operator[](int i) { return _array[i]; }
  ATTRIBUTE const T& operator[](int i) const { return _array[i]; }
  #endif

  ATTRIBUTE vec3& operator=(const vec3& a) {
    this->x = a.x;
    this->y = a.y;
    this->z = a.z;
    return *this;
  }

  template <typename U>
  ATTRIBUTE inline vec3<T, dtype>& operator=(const vec3<U, dtype>& a) {
    this->x = static_cast<T>(a.x);
    this->y = static_cast<T>(a.y);
    this->z = static_cast<T>(a.z);
    return *this;
  }

  template <typename U>
  ATTRIBUTE inline vec3<T, dtype>& operator=(vec3<U, dtype> a) {
    this->x = static_cast<T>(a.x);
    this->y = static_cast<T>(a.y);
    this->z = static_cast<T>(a.z);
    return *this;
  }

  template <typename U>
  ATTRIBUTE inline vec3<T, dtype>& operator+=(const vec3<U, dtype>& a) {
    this->x += a.x;
    this->y += a.y;
    this->z += a.z;
    return *this;
  }

  template <typename U>
  ATTRIBUTE inline vec3<T, dtype>& operator-=(const vec3<U, dtype>& a) {
    this->x -= a.x;
    this->y -= a.y;
    this->z -= a.z;
    return *this;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE inline vec3<T, dtype>& operator*=(const U& a) {
    this->x *= a;
    this->y *= a;
    this->z *= a;
    return *this;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE inline vec3<T, dtype>& operator/=(const U& a) {
    this->x /= a;
    this->y /= a;
    this->z /= a;
    return *this;
  }

  ATTRIBUTE inline bool operator==(const vec3<T, dtype>& a) const {
    bool flag = true;
    if (fabs(this->x - a.x) > 1.0e-15) {
      flag &= false;
    }
    if (fabs(this->y - a.y) > 1.0e-15) {
      flag &= false;
    }
    if (fabs(this->z - a.z) > 1.0e-15) {
      flag &= false;
    }
    return flag;
  }

  ATTRIBUTE inline bool operator!=(const vec3<T, dtype>& a) const {
    return !(*this == a);
  }

  ATTRIBUTE inline bool operator<(const vec3<T, dtype>& a) const {
    bool flag = true;
    if (this->x >= a.x) {
      flag &= false;
    }
    if (this->y >= a.y) {
      flag &= false;
    }
    if (this->z >= a.z) {
      flag &= false;
    }
    return flag;
  }

  ATTRIBUTE inline bool operator<=(const vec3<T, dtype>& a) const {
    bool flag = true;
    if (this->x > a.x) {
      flag &= false;
    }
    if (this->y > a.y) {
      flag &= false;
    }
    if (this->z > a.z) {
      flag &= false;
    }
    return flag;
  }

  ATTRIBUTE inline bool operator>(const vec3<T, dtype>& a) const {
    bool flag = true;
    if (this->x <= a.x) {
      flag &= false;
    }
    if (this->y <= a.y) {
      flag &= false;
    }
    if (this->z <= a.z) {
      flag &= false;
    }
    return flag;
  }

  ATTRIBUTE inline bool operator>=(const vec3<T, dtype>& a) const {
    bool flag = true;
    if (this->x < a.x) {
      flag &= false;
    }
    if (this->y < a.y) {
      flag &= false;
    }
    if (this->z < a.z) {
      flag &= false;
    }
    return flag;
  }

  template <typename U>
  ATTRIBUTE inline auto dot(const vec3<U, dtype>& a) const {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    return static_cast<rtype>(this->x) * a.x + static_cast<rtype>(this->y) * a.y + static_cast<rtype>(this->z) * a.z;
  }

  ATTRIBUTE inline dtype norm() const {
    return sqrt(this->dot(*this));
  }

  ATTRIBUTE inline dtype norm2() const {
    return this->dot(*this);
  }

  ATTRIBUTE inline vec3<T, dtype>& normalize() {
    dtype a = this->norm();
    this->x /= a;
    this->y /= a;
    this->z /= a;
    return *this;
  }

  template <typename U>
  ATTRIBUTE inline auto cross(const vec3<U, dtype>& a) const {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = static_cast<rtype>(this->y) * a.z - static_cast<rtype>(this->z) * a.y;
    c.y = static_cast<rtype>(this->z) * a.x - static_cast<rtype>(this->x) * a.z;
    c.z = static_cast<rtype>(this->x) * a.y - static_cast<rtype>(this->y) * a.x;
    return c;
  }

  template <typename U>
  ATTRIBUTE inline dtype dist(const vec3<U, dtype>& a) const {
    return (*this - a).norm();
  }

  friend inline std::ostream& operator<<(std::ostream& os, const vec3<T, dtype>& a) {
    os << "[" << a.x << ", " << a.y << ", " << a.z << "]";
    return os;
  }

  friend inline std::istream& operator>>(std::istream& is, vec3<T, dtype>& a) {
    is >> a.x >> a.y >> a.z;
    return is;
  }

  /*** Definition of friend functions ***/
  template <typename U>
  ATTRIBUTE friend inline auto operator+(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a.x + b.x;
    c.y = a.y + b.y;
    c.z = a.z + b.z;
    return c;
  }

  template <typename U>
  ATTRIBUTE friend inline auto operator-(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a.x - b.x;
    c.y = a.y - b.y;
    c.z = a.z - b.z;
    return c;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE friend inline auto operator*(const vec3<T, dtype>& a, const U& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a.x * b;
    c.y = a.y * b;
    c.z = a.z * b;
    return c;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE friend inline auto operator*(const U& a, const vec3<T, dtype>& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a * b.x;
    c.y = a * b.y;
    c.z = a * b.z;
    return c;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE friend inline auto operator/(const vec3<T, dtype>& a, const U& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a.x / b;
    c.y = a.y / b;
    c.z = a.z / b;
    return c;
  }

  template <typename U, std::enable_if_t<std::is_arithmetic_v<U>, int> = 0>
  ATTRIBUTE friend inline auto operator/(const U& a, const vec3<T, dtype>& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a / b.x;
    c.y = a / b.y;
    c.z = a / b.z;
    return c;
  }

  template <typename U>
  ATTRIBUTE friend inline auto operator^(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    using rtype = std::conditional_t<std::is_floating_point_v<T>, T, U>;
    vec3<rtype, dtype> c;
    c.x = a.x * b.x;
    c.y = a.y * b.y;
    c.z = a.z * b.z;
    return c;
  }

  template <typename U>
  ATTRIBUTE friend inline dtype dot(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    return a.dot(b);
  }

  ATTRIBUTE friend inline dtype norm(const vec3<T, dtype>& a) {
    return a.norm();
  }

  ATTRIBUTE friend inline dtype norm2(const vec3<T, dtype>& a) {
    return a.norm2();
  }

  ATTRIBUTE friend inline vec3<T, dtype> normalize(vec3<T, dtype>& a) {
    dtype norm = a.norm();
    a /= norm;
    return a;
  }

  template <typename U>
  ATTRIBUTE friend inline auto cross(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    return a.cross(b);
  }

  template <typename U>
  ATTRIBUTE friend inline dtype dist(const vec3<T, dtype>& a, const vec3<U, dtype>& b) {
    return a.dist(b);
  }
};