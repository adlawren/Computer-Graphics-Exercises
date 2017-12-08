
// Note: implemented using the following sources for reference:

// http://www.lighthouse3d.com/tutorials/glsl-12-tutorial/directional-lights-i/

// A super simple vertex shader that just mimics exactly what OpenGL's
// default fixed pipeline does

uniform float currentTime;

void main()
{
	gl_TexCoord[0] = gl_MultiTexCoord0;

	mat2 rotationMatrix;
	rotationMatrix[0] = vec2(cos(currentTime), sin(currentTime));
	rotationMatrix[1] = vec2(-sin(currentTime), cos(currentTime));
	gl_TexCoord[0].st = rotationMatrix * gl_MultiTexCoord0.st;

	vec3 vertexNormal = normalize(gl_NormalMatrix * gl_Normal);
	vec3 lightPosition = normalize(vec3(gl_LightSource[0].position));

	vec4 diffuseLighting = max(dot(vertexNormal, lightPosition), 0.0) * gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;
	vec4 ambientLighting = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;
	vec4 globalAmbientLighting = gl_LightModel.ambient * gl_FrontMaterial.ambient;

	gl_FrontColor = diffuseLighting + ambientLighting + globalAmbientLighting;

	gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
}

