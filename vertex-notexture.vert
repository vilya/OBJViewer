varying vec3 Ka, Kd;
varying vec3 normal, lightDir, halfVector;


void main()
{
  float NdotL, NdotHV;
  vec3 globalAmbient, materialAmbient;
  
  normal = normalize(gl_NormalMatrix * gl_Normal);
  lightDir = vec3(0, 0, 1);
  halfVector = normalize(gl_LightSource[0].halfVector.xyz);

  globalAmbient = gl_LightModel.ambient.rgb * gl_FrontMaterial.ambient.rgb;
  materialAmbient = gl_LightSource[0].ambient.rgb * gl_FrontMaterial.ambient.rgb;

  Ka = globalAmbient + materialAmbient;
  Kd = gl_FrontMaterial.diffuse.rgb * gl_LightSource[0].diffuse.rgb;

  gl_Position = ftransform();
}

