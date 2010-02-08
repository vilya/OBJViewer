varying vec3 Ka, Kd;
varying vec3 normal, lightDir, halfVector;


void main()
{
  float NdotL, NdotHV;
  vec3 globalAmbient, materialAmbient;
  
  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = normalize(vec3(gl_LightSource[0].position));
  halfVector = normalize(gl_LightSource[0].halfVector.xyz);

  globalAmbient = gl_LightModel.ambient.rgb * gl_FrontMaterial.ambient.rgb;
  materialAmbient = gl_LightSource[0].ambient.rgb * gl_FrontMaterial.ambient.rgb;

  Ka = globalAmbient + materialAmbient;
  Kd = gl_FrontMaterial.diffuse.rgb * gl_LightSource[0].diffuse.rgb;

  gl_TexCoord[0] = gl_MultiTexCoord0;
  gl_TexCoord[1] = gl_MultiTexCoord1;
  gl_TexCoord[2] = gl_MultiTexCoord2;
  gl_TexCoord[3] = gl_MultiTexCoord3;

  gl_Position = ftransform();
}

