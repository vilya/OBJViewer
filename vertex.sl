varying vec3 normal, lightDir;

void main()
{  
  lightDir = vec3(0, 1, 0);
  normal = normalize(gl_NormalMatrix * gl_Normal);

  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_TexCoord[2] = gl_MultiTexCoord2;
  gl_TexCoord[3] = gl_MultiTexCoord3;

  gl_Position = ftransform();
}
