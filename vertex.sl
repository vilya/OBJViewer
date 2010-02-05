// simple toon vertex shader
// www.lighthouse3d.com


varying vec3 normal, lightDir;

void main()
{	
	lightDir = vec3(0, 1, 0);
	normal = normalize(gl_NormalMatrix * gl_Normal);
	gl_Position = ftransform();
}
