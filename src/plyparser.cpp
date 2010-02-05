#include <cstdio>
#include "ply.h"  // From the thirdparty directory.

#include "model.h"
#include "plyparser.h"


//
// INTERNAL TYPES
//

struct PLYVertex {
  float x, y, z;
  float u, v;
  float nx, ny, nz;
  float r, g, b;
  void* otherData;
};

struct PLYFace {
  unsigned char nverts;    /* number of vertex indices in list */
  int *verts;              /* vertex index list */
  void* otherData;
};


//
// GLOBAL VARIABLES
//

PlyProperty vertexProps[] = {
  { "x",      PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, x), 0, 0, 0, 0 },
  { "y",      PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, y), 0, 0, 0, 0 },
  { "z",      PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, z), 0, 0, 0, 0 },
  { "u",      PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, u), 0, 0, 0, 0 },
  { "v",      PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, v), 0, 0, 0, 0 },
  { "nx",     PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex,nx), 0, 0, 0, 0 },
  { "ny",     PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex,ny), 0, 0, 0, 0 },
  { "nz",     PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex,nz), 0, 0, 0, 0 },
  { "red",    PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, r), 0, 0, 0, 0 },
  { "green",  PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, g), 0, 0, 0, 0 },
  { "blue",   PLY_FLOAT, PLY_FLOAT, offsetof(PLYVertex, b), 0, 0, 0, 0 }
};


PlyProperty faceProps[] = { /* list of property information for a face */
  { "vertex_indices", PLY_INT, PLY_INT, offsetof(PLYFace, verts),
    1, PLY_CHAR, PLY_CHAR, offsetof(PLYFace, nverts) }
};


bool hasTexCoords = false;
bool hasNormals = false;
bool hasColors = false;


//
// INTERNAL FUNCTIONS
//

void plyParseFaces(ParserCallbacks* callbacks, PlyFile* plySrc,
                   char* sectionName, int sectionSize, int numProperties)
  throw(ParseException)
{
}


//
// PUBLIC FUNCTIONS
//

void loadPLY(ParserCallbacks* callbacks, const char* path) throw(ParseException)
{
  int numElements = 0;
  char** elementNames = NULL;
  int fileType = 0;
  float version = 0.0;
  PlyFile* plySrc = ply_open_for_reading(const_cast<char*>(path),
      &numElements, &elementNames, &fileType, &version);

  for (int i = 0; i < numElements; ++i) {
    char* sectionName = elementNames[i];
    int sectionSize = 0;
    int numProperties = 0;

    PlyProperty** sectionProperties = ply_get_element_description(
        plySrc, sectionName, &sectionSize, &numProperties);

    if (strcmp("vertex", sectionName) == 0) {
      ply_get_property(plySrc, sectionName, &vertexProps[0]); 
      ply_get_property(plySrc, sectionName, &vertexProps[1]); 
      ply_get_property(plySrc, sectionName, &vertexProps[2]);

      // If there are any texture or normal coords, grab them too.
      unsigned int propMask = 0;
      for (int i = 0; i < numProperties; ++i) {
        PlyProperty* availableProp = sectionProperties[i];
        for (int j = 3; j < 11; ++j) {
          PlyProperty* requestedProp = &vertexProps[j];
          if (strcmp(requestedProp->name, availableProp->name) == 0) {
            ply_get_property(plySrc, sectionName, requestedProp);
            propMask |= (1 << j);
          }
        }
      }
      ply_get_other_properties(plySrc, sectionName, offsetof(PLYVertex, otherData));

      hasTexCoords = propMask & (0x3 << 3); // true if the u and v bits are set.
      hasNormals = propMask & (0x7 << 5); // true if the nx, ny and nz bits are set.
      hasColors = propMask & (0x7 << 8); // true if the r, g and b bits are set.
  
      for (int vertexNum = 0; vertexNum < sectionSize; ++vertexNum) {
        PLYVertex plyVert;
        ply_get_element(plySrc, &plyVert);

        callbacks->coordParsed(Float4(plyVert.x, plyVert.y, plyVert.z, 1.0));
        if (hasTexCoords)
          callbacks->texCoordParsed(Float4(plyVert.u, plyVert.v, 0.0, 1.0));
        if (hasNormals)
          callbacks->normalParsed(Float4(plyVert.nx, plyVert.ny, plyVert.nz, 1.0));
        // TODO: if (hasColors) { ... }
      }
    } else if (strcmp("face", sectionName) == 0) {
      ply_get_property(plySrc, sectionName, &faceProps[0]);
      ply_get_other_properties(plySrc, sectionName, offsetof(PLYFace, otherData));

      for (int i = 0; i < sectionSize; ++i) {
        PLYFace plyFace;
        ply_get_element(plySrc, &plyFace);

        Face* face = new Face();
        for (int j = 0; j < plyFace.nverts; ++j) {
          int v = plyFace.verts[j];
          int vt = hasTexCoords ? v : -1;
          int vn = hasNormals ? v : -1;
          face->vertexes.push_back(Vertex(v, vt, vn));
        }
        callbacks->faceParsed(face);
      }
    } else {
      ply_get_other_element(plySrc, sectionName, sectionSize);
    }
  }

  ply_close(plySrc);
}

