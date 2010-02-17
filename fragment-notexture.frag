varying vec3 Ka, Kd;
varying vec3 normal, lightDir, halfVector;


void main()
{
  vec3 n, halfV, color, Ks;
  float NdotL, NdotHV, D;

  n = normalize(normal);
  NdotL = max(dot(n, lightDir), 0.0);

  color = Ka;
  if (NdotL > 0.0) {
    color += NdotL * Kd;

    halfV = normalize(halfVector);
    NdotHV = max(dot(n, halfV), 0.0);

    Ks = gl_FrontMaterial.specular.rgb * gl_LightSource[0].specular.rgb *
        pow(NdotHV, gl_FrontMaterial.shininess);
    color += Ks;
  }

  gl_FragColor = vec4(color, 1.0);
}

