varying vec3 normal, lightDir;
uniform sampler2D mapKa, mapKd, mapKs, mapD;

void main()
{
  float intensity, D;
  vec3 Ka, Kd, Ks;
  vec4 dissolve;

  intensity = max(dot(lightDir, normal), 0.0);

  Ka = gl_FrontMaterial.ambient.rgb  * texture2D(mapKa, gl_TexCoord[0].st);
  Kd = gl_FrontMaterial.diffuse.rgb  * texture2D(mapKd, gl_TexCoord[1].st) * intensity;
  Ks = gl_FrontMaterial.specular.rgb * texture2D(mapKs, gl_TexCoord[2].st) * intensity;

  D = gl_FrontMaterial.alpha;
  dissolve = texture2D(mapD, gl_TexCoord[3].st);

  gl_FragColor = float4(Ka + intensity * (Kd + Ks), D * dissolve.a); 
}
