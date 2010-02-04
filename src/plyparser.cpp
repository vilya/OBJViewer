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
  { "x",      Float32, Float32, offsetof(PLYVertex, x), 0, 0, 0, 0 },
  { "y",      Float32, Float32, offsetof(PLYVertex, y), 0, 0, 0, 0 },
  { "z",      Float32, Float32, offsetof(PLYVertex, z), 0, 0, 0, 0 },
  { "u",      Float32, Float32, offsetof(PLYVertex, u), 0, 0, 0, 0 },
  { "v",      Float32, Float32, offsetof(PLYVertex, v), 0, 0, 0, 0 },
  { "nx",     Float32, Float32, offsetof(PLYVertex,nx), 0, 0, 0, 0 },
  { "ny",     Float32, Float32, offsetof(PLYVertex,ny), 0, 0, 0, 0 },
  { "nz",     Float32, Float32, offsetof(PLYVertex,nz), 0, 0, 0, 0 },
  { "red",    Float32, Float32, offsetof(PLYVertex, r), 0, 0, 0, 0 },
  { "green",  Float32, Float32, offsetof(PLYVertex, g), 0, 0, 0, 0 },
  { "blue",   Float32, Float32, offsetof(PLYVertex, b), 0, 0, 0, 0 }
};


PlyProperty faceProps[] = { /* list of property information for a face */
  { "vertex_indices", Int32, Int32, offsetof(PLYFace, verts),
    1, Uint8, Uint8, offsetof(PLYFace, nverts) }
};


bool hasTexCoords = false;
bool hasNormals = false;
bool hasColors = false;


//
// INTERNAL FUNCTIONS
//

void plyParseVertices(ParserCallbacks* callbacks, PlyFile* plySrc) throw(ParseException)
{
  // We always need to read the xyz position.
  setup_property_ply(plySrc, &vertexProps[0]); 
  setup_property_ply(plySrc, &vertexProps[1]); 
  setup_property_ply(plySrc, &vertexProps[2]);

  // If there are any texture or normal coords, grab them too.
  unsigned int propMask = 0;
  for (int i = 0; i < plySrc->which_elem->nprops; ++i) {
    PlyProperty* availableProp = plySrc->which_elem->props[i];
    for (int j = 3; j < 11; ++j) {
      PlyProperty* requestedProp = &vertexProps[j];
      if (strcmp(requestedProp->name, availableProp->name) == 0) {
        setup_property_ply(plySrc, requestedProp);
        propMask |= (1 << j);
      }
    }
  }
  get_other_properties_ply(plySrc, offsetof(PLYVertex, otherData));

  hasTexCoords = propMask & (0x3 << 3); // true if the u and v bits are set.
  hasNormals = propMask & (0x7 << 5); // true if the nx, ny and nz bits are set.
  hasColors = propMask & (0x7 << 8); // true if the r, g and b bits are set.

  for (int vertexNum = 0; vertexNum < plySrc->which_elem->num; ++vertexNum) {
    PLYVertex plyVert;
    get_element_ply(plySrc, &plyVert);

    callbacks->coordParsed(Float4(plyVert.x, plyVert.y, plyVert.z, 1.0));
    if (hasTexCoords)
      callbacks->texCoordParsed(Float4(plyVert.u, plyVert.v, 0.0, 1.0));
    if (hasNormals)
      callbacks->normalParsed(Float4(plyVert.nx, plyVert.ny, plyVert.nz, 1.0));
    // TODO: if (hasColors) { ... }
  }
}


void plyParseFaces(ParserCallbacks* callbacks, PlyFile* plySrc) throw(ParseException)
{
  setup_property_ply(plySrc, &faceProps[0]);
  get_other_properties_ply(plySrc, offsetof(PLYFace, otherData));

  for (int i = 0; i < plySrc->which_elem->num; ++i) {
    PLYFace plyFace;
    get_element_ply(plySrc, &plyFace);

    Face* face = new Face();
    for (int j = 0; j < plyFace.nverts; ++j) {
      int v = plyFace.verts[j];
      int vt = hasTexCoords ? v : -1;
      int vn = hasNormals ? v : -1;
      face->vertexes.push_back(Vertex(v, vt, vn));
    }
    callbacks->faceParsed(face);
  }
}


//
// PUBLIC FUNCTIONS
//

void loadPLY(ParserCallbacks* callbacks, const char* path) throw(ParseException)
{
  FILE* f = fopen(path, "rb");
  if (f == NULL)
    throw ParseException("Unable to open file %s.\n", path);

  PlyFile* plySrc = read_ply(f);
  int sectionSize;
  for (int i = 0; i < plySrc->num_elem_types; ++i) {
    char* sectionName = setup_element_read_ply(plySrc, i, &sectionSize);
    if (strcmp("vertex", sectionName) == 0)
      plyParseVertices(callbacks, plySrc);
    else if (strcmp("face", sectionName) == 0)
      plyParseFaces(callbacks, plySrc);
    else
      get_other_element_ply(plySrc);
  }

  close_ply(plySrc);
}

