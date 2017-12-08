
uniform sampler2D textureSample_0;
uniform sampler2D textureSample_1;
uniform float currentTime;

void main()
{
	gl_FragColor = texture2D(textureSample_0, gl_TexCoord[0].st);
	gl_FragColor *= gl_Color;

	// add lightmap component
	vec4 lightMapValue = texture2D(textureSample_1, gl_TexCoord[0].st);
	gl_FragColor += lightMapValue * vec4(1.0, 0.0, 0.0, 1.0) * abs(sin(currentTime));
}
