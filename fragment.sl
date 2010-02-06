varying vec3 normal, lightDir;
uniform sampler2D mapKa, mapKd, mapKs, mapD;


void main()
{
  float intensity, D;
  vec3 Ka, Kd, Ks;
  vec4 dissolve, col;

  intensity = max(dot(lightDir, normal), 0.0);

  Ka = gl_FrontMaterial.ambient.rgb  * texture2D(mapKa, gl_TexCoord[0].st).rgb;
  Kd = gl_FrontMaterial.diffuse.rgb  * texture2D(mapKd, gl_TexCoord[1].st).rgb;
  Ks = gl_FrontMaterial.specular.rgb * texture2D(mapKs, gl_TexCoord[2].st).rgb;
  D  = gl_FrontMaterial.diffuse.a    * texture2D(mapD, gl_TexCoord[3].st).a;

  gl_FragColor = vec4(Kd, D);
}
