/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(location = 1) uniform int iTime;
uniform sampler2D accTex;

#define ITER_DIST 16
#define saturate(x) (clamp((x), 0.0, 1.0))


float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

// Based on Morgan McGuire @morgan3d
// https://www.shadertoy.com/view/4dS3Wd
float noise (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
            (c - a)* u.y * (1.0 - u.x) +
            (d - b) * u.x * u.y;
}

float fbm (in vec2 st, float amp) {
    // Initial values
    float value = 0.0;
    float amplitude = amp;
    
    // Loop of octaves
    for (int i = 0; i < 4; i++) {
        value += amplitude * noise(st);
        st *= 4.3;
        amplitude *= .5;
    }
    return value;
}

// RGB ramp stolen from Ferris
vec3 aberrationColor(float f)
{
	f = f * 3.0 - 1.5;
	return saturate(vec3(-f, 1.0 - abs(f), f));
}

vec2 distort(vec2 uv, float i) {
    float dx = (0.1 - fbm(vec2(uv.y*4.0, float(iTime)/4000.0), i))/4.0;
    return vec2(uv.x + dx, uv.y);
}

vec3 standard_excess_like_aberation(vec2 uv) {
    const float step_size = 1.0 / (float(ITER_DIST) - 1.0);
    float t = step_size; //* hash2(uv + sin(iTime)); // We pseudo randomize the step to have some dithering pattern.

    vec3 sum_color = vec3(0.0);
    vec3 sum_weight = vec3(0.0);
    for (int i = 0; i < ITER_DIST; ++i) {
	    vec3 weight = aberrationColor(t);
        sum_weight += weight;
        sum_color += weight 
            * texture(accTex, distort(uv, 0.1+(float(i)/float(ITER_DIST))/32.0)).rgb;
        t += step_size;
    }

    sum_color /= sum_weight;
    return sum_color;
}

void main()
{
	// readback the buffer
	vec3 abberation = standard_excess_like_aberation(gl_FragCoord.xy/iResolution.xy);
    vec3 tex = texture(accTex, gl_FragCoord.xy/iResolution.xy).rgb;

	// divide accumulated color by the sample count
    float x = float(iTime)/1000.0;
    float mix_val = (1.0+sin(5.0*sin(2.0*sin(4.0*sin(x))))*sin(8.0*x)*sin(x/3.0))/2.0;
	vec3 color = tex*mix_val + abberation*(1.0-mix_val);

	/* perform any post-processing you like here */

	// for example, some B&W with an S-curve for harsh contrast
	//color = smoothstep(0.,1.,color.ggg);

	// present for display
	gl_FragColor = vec4(color, 1);
}
