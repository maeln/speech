/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(location = 1) uniform int iTime;
uniform sampler2D sdfTex;

void main()
{
	//vec4 tex = texture(texSampler, gl_FragCoord.xy/iResolution.xy);
	vec4 tex = texelFetch(sdfTex, ivec2(gl_FragCoord.xy), 0);
	gl_FragColor = tex;
}
