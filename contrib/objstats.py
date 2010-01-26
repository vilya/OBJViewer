#!/usr/bin/env python
import sys


def main():
  for model in sys.argv[1:]:
    low = [10000.0] * 3
    high = [-10000.0] * 3

    faces = 0
    vertexes = 0
    texture_vertexes = 0
    param_vertexes = 0
    normals = 0

    biggest_face = 0

    print "--"
    print model
    with open(sys.argv[1]) as f:
      for line in f:
        if line.startswith('f ') or line.startswith('fo '):
          faces += 1
          face_size = len(line.split()) - 1
          if face_size > biggest_face:
            biggest_face = face_size
        elif line.startswith('v '):
          vertexes += 1
          v = map(float, line[2:].split())
          low = [min(*c) for c in zip(low, v)]
          high = [max(*c) for c in zip(high, v)]
        elif line.startswith('vt '):
          texture_vertexes += 1
        elif line.startswith('vp '):
          param_vertexes += 1
        elif line.startswith('vn '):
          normals += 1

    for result in zip( ('x', 'y', 'z'), low, high ):
      print "%s = %f - %f" % tuple(result)
    print ""
    print "%d faces of up to %d vertices" % (faces, biggest_face)
    print vertexes, "vertexes"
    print texture_vertexes, "texture vertexes"
    print param_vertexes, "parameter vertexes"
    print normals, "normals"


if __name__ == '__main__':
  main()

