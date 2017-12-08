
uniform sampler2D textureSample_0;
uniform sampler2D textureSample_1;

in vec2 scaledTextureCoordinates;

void main()
{
	vec4 sandTexture = texture2D(textureSample_0, gl_TexCoord[0].st);
	vec4 scaledSandTexture = texture2D(textureSample_0, scaledTextureCoordinates);
	vec4 grassTexture = texture2D(textureSample_1, gl_TexCoord[0].st);
	vec4 scaledGrassTexture = texture2D(textureSample_1, scaledTextureCoordinates);

	sandTexture.rgb = scaledSandTexture.rgb;
	grassTexture.rgb = scaledGrassTexture.rgb;

	gl_FragColor = (grassTexture * grassTexture.a) + (sandTexture * (1.0 - grassTexture.a));
	gl_FragColor *= gl_Color;
}
