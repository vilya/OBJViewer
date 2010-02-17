varying vec3 Ka, Kd;
varying vec3 normal, lightDir, halfVector;
uniform sampler2D mapKa, mapKd, mapKs, mapD;
uniform bool hasMapKa, hasMapKd, hasMapKs, hasMapD;


void main()
{
  vec3 n, halfV, color, Ks;
  float NdotL, NdotHV, D;

  n = normalize(normal);
  NdotL = max(dot(n, lightDir), 0.0);

  if (hasMapKa)
    color = Ka * texture2D(mapKa, gl_TexCoord[0].st).rgb;
  else
    color = Ka;

  if (NdotL > 0.0) {
    if (hasMapKd)
      color += NdotL * Kd * texture2D(mapKd, gl_TexCoord[1].st).rgb;
    else
      color += NdotL * Kd;

    halfV = normalize(halfVector);
    NdotHV = max(dot(n, halfV), 0.0);

    Ks = gl_FrontMaterial.specular.rgb * gl_LightSource[0].specular.rgb *
        pow(NdotHV, gl_FrontMaterial.shininess);
    if (hasMapKs)
      color += Ks * texture2D(mapKs, gl_TexCoord[2].st).rgb;
    else
      color += Ks;
  }

  if (hasMapD)
    D = gl_FrontMaterial.diffuse.a * texture2D(mapD,  gl_TexCoord[3].st).a;
  else
    D = gl_FrontMaterial.diffuse.a;

  gl_FragColor = vec4(color, D);
}

