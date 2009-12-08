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


#endif // OBJViewer_math3d_h

