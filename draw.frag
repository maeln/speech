/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(location = 1) uniform int iTime;

#define MAXSTEPS 128
#define MINDIST  0.0005
#define MAXDIST  20.0
#define saturate(x) (clamp(0.0, 1.0, x))

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

float fbm (in vec2 st) {
    // Initial values
    float value = 0.0;
    float amplitude = 0.5;
    //
    // Loop of octaves
    for (int i = 0; i < 5; i++) {
        value += amplitude * noise(st);
        st *= 2.;
        amplitude *= .5;
    }
    return value;
}

mat4 rotationMatrix(vec3 axis, float angle)
{
    axis = normalize(axis);
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
    
    return mat4(oc * axis.x * axis.x + c,           oc * axis.x * axis.y - axis.z * s,  oc * axis.z * axis.x + axis.y * s,  0.0,
                oc * axis.x * axis.y + axis.z * s,  oc * axis.y * axis.y + c,           oc * axis.y * axis.z - axis.x * s,  0.0,
                oc * axis.z * axis.x - axis.y * s,  oc * axis.y * axis.z + axis.x * s,  oc * axis.z * axis.z + c,           0.0,
                0.0,                                0.0,                                0.0,                                1.0);
}

// Primitive fun from Iq: 
// http://iquilezles.org/www/articles/distfunctions/distfunctions.htm

struct pLight {
    vec3 position;
    vec3 ambiant;
    vec3 diffuse;
    vec3 specular;
};

float smin( float a, float b, float k )
{
    float h = clamp( 0.5+0.5*(b-a)/k, 0.0, 1.0 );
    return mix( b, a, h ) - k*h*(1.0-h);
}

float sdBox( vec3 p, vec3 b )
{
    return length(max(abs(p)-b,0.0));
}


float sdPlane(vec3 p)
{
  return p.y;
}

float sphere(vec3 p, float s)
{
    return length(p) - s;
}	

float sdHexPrism( vec3 p, vec2 h )
{
    vec3 q = abs(p);
    return max(q.z-h.y,max((q.x*0.866025+q.y*0.5),q.y)-h.x);
}

vec2 opU( vec2 d1, vec2 d2 )
{
	return (d1.x<d2.x) ? d1 : d2;
}


vec2 spheres(vec3 p)
{
    //p.z = max(p.z, 0.0);
   	vec3 c = vec3(2.0, 2.0, 2.0);
    vec3 q = mod(p+0.5*c,c)-0.5*c;
    q = (rotationMatrix(vec3(1.0, 0.0, 0.0), (float(iTime)/1000.0)) * vec4(q, 1.0)).xyz;
    float s1 = sdBox(q-vec3(0,0,0), vec3(0.5));
    float s2 = sphere(q-vec3(0,1.0,1.0), 0.5);
    vec2 u1 = vec2(smin(s1,s2,0.3), 1.0);
    
    return vec2(s1, 1.0);
}

vec2 centralSphere(vec3 p) {
    float centralSphere = sphere(p - vec3(1.0, 1.0+sin((float(iTime)/1000.0))*sin((float(iTime)/1000.0)*2.0), cos(cos(cos((float(iTime)/1000.0))*0.3)*2.0)), 0.5);
    
    return vec2(centralSphere, 2.0);
}

vec2 scene(vec3 ray)
{
    return spheres(ray);
}

vec2 DE(vec3 ray) {
    return scene(ray);
}


vec3 normal(vec3 pos) {
    vec2 eps = vec2(0.0, MINDIST);
	return normalize(vec3(
    DE(pos + eps.yxx).x - DE(pos - eps.yxx).x,
    DE(pos + eps.xyx).x - DE(pos - eps.xyx).x,
    DE(pos + eps.xxy).x - DE(pos - eps.xxy).x));
}

vec2 raymarch(vec3 from, vec3 direction)
{
    float t = 1.0*MINDIST;
    int i = 0;
    float obj = -1.0;
    for(int steps=0; steps<MAXSTEPS; ++steps)
    {
        ++i;
        vec2 dist = DE(from + t * direction);
        if(dist.x < MINDIST || t >= MAXDIST) break;
        t += dist.x;
        obj = dist.y;
    }
    
    return vec2(t, t > MAXDIST ? -1.0 : obj);
}

vec3 fog(vec3 sky, vec3 mat, float dist) {
    float fogAmount = 1.0 - min(exp(-dist*0.4), 1.0);
    return mat;
}

vec3 material(vec2 c, vec3 hit, vec3 sky) {
    vec3 color = sky;
    if(c.y < 0.0) return color;
    color = normalize(vec3(0.3*log2(c.x),0.2,0.8*(1.0-log2(c.x))));
    return fog(sky, color, c.x);
}

vec3 phong(vec3 hit, vec3 eye, vec3 N, pLight light, float ks) {
    vec3 L = normalize(light.position - hit);
    vec3 V = normalize(eye - hit);
    vec3 R = reflect(L, N);
    vec3 ambiant = light.ambiant;
    vec3 diffuse = max(dot(L,N), 0.0)*light.diffuse;
    vec3 specular = pow(max(dot(R,V), 0.0), ks)*light.specular;
    return ambiant + 0.5*(diffuse+specular);
}

float shininess(vec3 hit, vec3 eye, vec3 normal, pLight light) {
    float ks = 1.0; // Specular component, should be part of the material.
    vec3 L = light.position - hit;
    vec3 R = reflect(L, normal);
    vec3 V = eye - hit;
    return pow(dot(R, V), ks);
}

mat3 rotationX(float angle) {
	float s = sin(angle);
	float c = cos(angle);

	return mat3(1.0, 0.0, 0.0,
                0.0, c, s,
                0.0, -s, c);
}

void main()
{
    pLight l1 = pLight(vec3((float(iTime)/1000.0)-3.0, 2.0*sin((float(iTime)/1000.0)), cos((float(iTime)/1000.0))*3.0),
                       vec3(0.8), vec3(1.0, 0.0, 0.0), vec3(0.8, 0.0, 0.0));
    
   	pLight l2 = pLight(vec3((float(iTime)/1000.0)-3.0, -2.0, -3.0),
                       vec3(0.3), vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, 0.8));
    
    vec2 uv = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;
    
    vec2 uv2 = gl_FragCoord.xy / iResolution.xy;
    int tx = int(uv2.x*512.0);
    
    vec3 target  = vec3((float(iTime)/1000.0)-1.0, .5, 0.0);
	vec3 eye     = vec3((float(iTime)/1000.0), sin((float(iTime)/1000.0)), cos((float(iTime)/1000.0)));
    vec3 up      = vec3(0.0, sin((float(iTime)/1000.0)*0.5), cos((float(iTime)/1000.0)*0.5));
    target = eye + vec3(1.0, 0.0, 0.0);
    
    vec3 eyeDir   = normalize(target - eye);
    vec3 eyeRight = normalize(cross(up, eye));
    vec3 eyeUp    = normalize(cross(eye, eyeRight));
    
    vec3 rayDir = normalize(eyeRight * uv.x + eyeUp * uv.y + eyeDir);
    
    vec3 hi = vec3(255.0, 122.0, 122.0)/255.0;
    vec3 lo = vec3(134.0, 22.0, 87.0)/255.0;
    vec3 color = mix(lo, hi, gl_FragCoord.y/iResolution.y);
    vec3 sky = color;
    vec2 c = raymarch(eye, rayDir);
    vec3 hit = eye+c.x*rayDir;
    vec3 norm = normal(hit);
    
    if(c.y >= 0.0) {
        color = material(c, hit, color);
        color = color * phong(hit, eye, norm, l1, 2.0);
    }
    
	gl_FragColor = vec4(color, c.x/MAXDIST);
}
