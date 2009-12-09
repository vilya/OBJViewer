#include <cmath>
#include <cstdio>

#include "math3d.h"


static const float EPSILON = 0.00001f;


static int assertionsFailed = 0;


void assertEqual(float expected, float actual, const char* failMessage = NULL)
{
  if (fabsf(expected - actual) > EPSILON) {
    ++assertionsFailed;
    fprintf(stderr, "Assertion failed: [%f != %f]", expected, actual);
    if (failMessage != NULL)
      fprintf(stderr, ": %s\n", failMessage);
    else
      fprintf(stderr, ".\n");
  }
}


int main(int argc, char** argv)
{
  Matrix4 m = Matrix4::rotateX(90);
  Float4 v(0, 1, 0);
  
  Float4 r = m * v;
  assertEqual(0.0f, r.x, "x has changed.");
  assertEqual(0.0f, r.y, "y should be zero after rotation.");
  assertEqual(1.0f, r.z, "z should be 1 after rotation.");

  if (assertionsFailed > 0)
    printf("Test failed.\n");
  else
    printf("Test passed.\n");
  return assertionsFailed;
}

