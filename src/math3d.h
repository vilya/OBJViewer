#ifndef OBJViewer_math3d_h
#define OBJViewer_math3d_h


struct Float4 {
  union {
    struct { float x, y, z, w; };
    struct { float r, g, b, a; };
    float data[4];
  };

  Float4();
  Float4(float _x, float _y, float _z, float _w = 1.0);
  Float4(const Float4& vec);

  float operator [] (int index) const;
  float& operator [] (int index);
};

Float4 operator + (const Float4& a, const Float4& b);
Float4 operator - (const Float4& a, const Float4& b);

Float4 operator / (const Float4& a, float k);


struct Matrix4 {
  union {
    struct { float a00, a01, a02, a03,
                   a10, a11, a12, a13,
                   a20, a21, a22, a23,
                   a30, a31, a32, a33; };
    float data[4][4];
  };

  Matrix4();

  static Matrix4 identity();
  static Matrix4 rotateX(float degrees);
  static Matrix4 rotateY(float degrees);
  static Matrix4 rotateZ(float degrees);
};


Float4 operator * (const Float4& v, const Matrix4& m);
Float4 operator * (const Matrix4& m, const Float4& v);

#endif // OBJViewer_math3d_h

