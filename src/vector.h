#ifndef vh_vector_h
#define vh_vector_h

namespace vh {

  //
  // Forward declarations
  //

  struct Vector2;
  struct Vector3;
  struct Vector4;
  struct Matrix3;
  struct Matrix4;


  //
  // Types
  //

  struct Vector2 {
    union {
      struct { float x, y; };
      struct { float u, v; };
      float data[2];
    };

    Vector2();
    Vector2(float ix, float iy);
    Vector2(const Vector2& v);
  };


  struct Vector3 {
    union {
      struct { float x, y, z; };
      struct { float u, v, w; };
      struct { float r, g, b; };
      float data[3];
    };

    Vector3();
    Vector3(float ix, float iy, float iz);
    Vector3(const Vector2& v, float iz);
    Vector3(const Vector3& v);
  };


  struct Vector4 {
    union {
      struct { float x, y, z, w; };
      struct { float u, v, s, t; };
      struct { float r, g, b, a; };
      float data[4];
    };

    Vector4();
    Vector4(float ix, float iy, float iz, float iw);
    Vector4(const Vector2& v, float iz, float iw);
    Vector4(const Vector3& v, float iw);
    Vector4(const Vector4& v);
  };


  struct Matrix3 {
    union {
      struct { float m00, m01, m02,
                     m10, m11, m12,
                     m20, m21, m22; };
      float data[3][3];
    };

    Matrix3();
    Matrix3(const Matrix3& m);

    static Matrix3 identity();
    static Matrix3 rows(const Vector3& a, const Vector3& b, const Vector3& c);
    static Matrix3 columns(const Vector3& a, const Vector3& b, const Vector3& c);

    static Matrix3 rotationX(float degrees);
    static Matrix3 rotationY(float degrees);
    static Matrix3 rotationZ(float degrees);

    static Matrix3 translation(float x, float y);
    static Matrix3 scale(float sx, float sy, float sz);
  };


  struct Matrix4 {
    union {
      struct { float m00, m01, m02, m03,
                     m10, m11, m12, m13,
                     m20, m21, m22, m23,
                     m30, m31, m32, m33; };
      float data[4][4];
    };

    Matrix4();
    Matrix4(const Matrix3& m);
    Matrix4(const Matrix4& m);

    static Matrix4 identity();
    static Matrix4 rows(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);
    static Matrix4 columns(const Vector4& a, const Vector4& b, const Vector4& c, const Vector4& d);

    static Matrix4 rotationX(float degrees);
    static Matrix4 rotationY(float degrees);
    static Matrix4 rotationZ(float degrees);
    static Matrix4 rotation(float degrees, float x, float y, float z);
    static Matrix4 translation(float x, float y, float z);
    static Matrix4 scale(float sx, float sy, float sz, float sw);
  };


  //
  // Functions
  //

  Vector2 operator + (const Vector2& lhs, const Vector2& rhs);
  Vector2 operator - (const Vector2& lhs, const Vector2& rhs);
  Vector2 operator * (const Vector2& lhs, const Vector2& rhs);
  Vector2 operator / (const Vector2& lhs, const Vector2& rhs);

  Vector2 operator * (const Vector2& lhs, float rhs);
  Vector2 operator * (float lhs, const Vector2& rhs);
  Vector2 operator / (const Vector2& lhs, float rhs);
  Vector2 operator / (float lhs, const Vector2& rhs);

  float sum(const Vector2& v);
  float dot(const Vector2& lhs, const Vector2& rhs);
  float min(const Vector2& v);
  float max(const Vector2& v);
  float lengthSqr(const Vector2& v);
  float length(const Vector2& v);

  Vector2 pow(const Vector2& v, float exponent);
  Vector2 clamp(const Vector2& v, float low, float high);
  Vector2 lowCorner(const Vector2& lhs, const Vector2& rhs);
  Vector2 highCorner(const Vector2& lhs, const Vector2& rhs);
  Vector2 norm(const Vector2& lhs);


  Vector3 operator + (const Vector3& lhs, const Vector3& rhs);
  Vector3 operator - (const Vector3& lhs, const Vector3& rhs);
  Vector3 operator * (const Vector3& lhs, const Vector3& rhs);
  Vector3 operator / (const Vector3& lhs, const Vector3& rhs);

  Vector3 operator * (const Vector3& lhs, float rhs);
  Vector3 operator * (float lhs, const Vector3& rhs);
  Vector3 operator / (const Vector3& lhs, float rhs);
  Vector3 operator / (float lhs, const Vector3& rhs);

  Vector3 operator * (const Vector3& lhs, const Matrix3& rhs);
  Vector3 operator * (const Matrix3& lhs, const Vector3& rhs);

  float sum(const Vector3& v);
  float dot(const Vector3& lhs, const Vector3& rhs);
  float min(const Vector3& v);
  float max(const Vector3& v);
  float lengthSqr(const Vector3& v);
  float length(const Vector3& v);

  Vector3 pow(const Vector3& v, float exponent);
  Vector3 cross(const Vector3& lhs, const Vector3& rhs);
  Vector3 clamp(const Vector3& v, float low, float high);
  Vector3 lowCorner(const Vector3& lhs, const Vector3& rhs);
  Vector3 highCorner(const Vector3& lhs, const Vector3& rhs);
  Vector3 norm(const Vector3& lhs);


  Vector4 operator + (const Vector4& lhs, const Vector4& rhs);
  Vector4 operator - (const Vector4& lhs, const Vector4& rhs);
  Vector4 operator * (const Vector4& lhs, const Vector4& rhs);
  Vector4 operator / (const Vector4& lhs, const Vector4& rhs);

  Vector4 operator * (const Vector4& lhs, float rhs);
  Vector4 operator * (float lhs, const Vector4& rhs);
  Vector4 operator / (const Vector4& lhs, float rhs);
  Vector4 operator / (float lhs, const Vector4& rhs);

  Vector4 operator * (const Vector4& lhs, const Matrix4& rhs);
  Vector4 operator * (const Matrix4& lhs, const Vector4& rhs);

  float sum(const Vector4& v);
  float dot(const Vector4& lhs, const Vector4& rhs);
  float min(const Vector4& v);
  float max(const Vector4& v);
  float lengthSqr(const Vector4& v);
  float length(const Vector4& v);

  Vector4 pow(const Vector4& v, float exponent);
  Vector4 cross(const Vector4& lhs, const Vector4& rhs);
  Vector4 clamp(const Vector4& v, float low, float high);
  Vector4 lowCorner(const Vector4& lhs, const Vector4& rhs);
  Vector4 highCorner(const Vector4& lhs, const Vector4& rhs);
  Vector4 norm(const Vector4& lhs);


  Matrix3 operator + (const Matrix3& lhs, const Matrix3& rhs);
  Matrix3 operator - (const Matrix3& lhs, const Matrix3& rhs);
  Matrix3 operator * (const Matrix3& lhs, const Matrix3& rhs);

  Matrix3 operator * (const Matrix3& lhs, float rhs);
  Matrix3 operator * (float lhs, const Matrix3& rhs);
  Matrix3 operator / (const Matrix3& lhs, float rhs);
  Matrix3 operator / (float lhs, const Matrix3& rhs);

  Matrix3 pow(const Matrix3& lhs, float exponent);
  float det(const Matrix3& m);

  bool invertible(const Matrix3& m);
  Matrix3 inverse(const Matrix3& m);

  Vector3 columnVec(const Matrix3& m, unsigned int col);
  Vector3 rowVec(const Matrix3& m, unsigned int row);


  Matrix4 operator + (const Matrix4& lhs, const Matrix4& rhs);
  Matrix4 operator - (const Matrix4& lhs, const Matrix4& rhs);
  Matrix4 operator * (const Matrix4& lhs, const Matrix4& rhs);
  Matrix4 operator / (const Matrix4& lhs, const Matrix4& rhs);

  Matrix4 operator * (const Matrix4& lhs, float rhs);
  Matrix4 operator * (float lhs, const Matrix4& rhs);
  Matrix4 operator / (const Matrix4& lhs, float rhs);
  Matrix4 operator / (float lhs, const Matrix4& rhs);

  Matrix4 pow(const Matrix4& lhs, float exponent);
  float det(const Matrix4& m);

  bool invertible(const Matrix4& m);
  Matrix4 inverse(const Matrix4& m);

  Vector4 columnVec(const Matrix4& m, unsigned int col);
  Vector4 rowVec(const Matrix4& m, unsigned int row);


} // namespace vh

#endif // vh_vector_h

