uniform sampler2D texture;
uniform sampler2D keyTexture;

void main()
{
	vec4 key = texture2D(keyTexture, gl_TexCoord[0].xy);
	if(key.a == 0.0) discard;
	
	gl_FragColor = gl_Color * texture2D(texture, gl_TexCoord[0].xy);
}