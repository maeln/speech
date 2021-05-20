/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(location = 1) uniform int iTime;

#define PI 3.1415
#define MAXSTEPS 128
#define MINDIST  0.0005
#define MAXDIST  20.0
#define saturate(x) (clamp(0.0, 1.0, x))

struct pLight {
    vec3 position;
    vec3 ambiant;
    vec3 diffuse;
    vec3 specular;
};

float sec = float(iTime)/1000.0;

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
}

float sphere(vec3 p, float s)
{
    return length(p) - s;
}

// Compute the position of particle {i} at the time {iTime}.
vec3 calcPos(int i) {
    float period = 5.0;
    float mTime = sec+(float(i)*0.2);
    
    // we change the direction of the particle every {period} seconds.
    float dv = floor(mTime/period+1.0);
    
    float dx = 0.5-random(vec2(i,i)*dv);
    float dy = 0.5-random(vec2(i-1,i+1)*dv);
    float dz = 0.5-random(vec2(i+1,i-1)*dv);
    
    vec3 dir = vec3(dx, dy, dz);
    
    // We multiply direction {dir} by the {iTime} modulo {period} so that we reset the particle position every {period} seconds 
    return dir * (mod(mTime,period)) * 2.0;
}

// Draw all the particles
vec2 scene(vec3 ray)
{
    float de = MAXDIST;
    float m = 0.0;
    for(int i=0; i<100; ++i) {
        // if we just want the distance 
        // de = min(sphere(ray-calcPos(i), 0.1), de);
        
        // So that we can return the "material id"
        // probably slower, but we can color each particle differently !
        float di = sphere(ray-calcPos(i), 0.1);
        if(di < de) {
            de = di;
            m = float(i);
        }
    }
    
    return vec2(de, m);
}

vec3 normal(vec3 pos) {
    vec2 eps = vec2(0.0, MINDIST);
	return normalize(vec3(
    scene(pos + eps.yxx).x - scene(pos - eps.yxx).x,
    scene(pos + eps.xyx).x - scene(pos - eps.xyx).x,
    scene(pos + eps.xxy).x - scene(pos - eps.xxy).x));
}

vec2 raymarch(vec3 from, vec3 direction)
{
    float t = 1.0*MINDIST;
    int i = 0;
    float obj = -1.0;
    for(int steps=0; steps<MAXSTEPS; ++steps)
    {
        ++i;
        vec2 dist = scene(from + t * direction);
        if(dist.x < MINDIST || t >= MAXDIST) break;
        t += dist.x;
        obj = dist.y;
    }
    
    return vec2(t, t > MAXDIST ? -1.0 : obj);
}

vec3 material(vec2 c, vec3 hit, vec3 sky) {
    vec3 color = sky;
    float r = random(vec2(c.y,c.y));
    float g = random(vec2(c.y-1.0,c.y+1.0));
    float b = random(vec2(c.y+1.0,c.y-1.0));
    return vec3(r,g,b);
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

float sdCircle(vec2 p, float r)
{
    return length(p) - r;
}

float sdArc( in vec2 p, in vec2 sca, in vec2 scb, in float ra, in float rb )
{
    p *= mat2(sca.x,sca.y,-sca.y,sca.x);
    p.x = abs(p.x);
    float k = (scb.y*p.x>scb.x*p.y) ? dot(p.xy,scb) : length(p);
    return sqrt(max(0.0, dot(p,p) + ra*ra - 2.0*ra*k)) - rb;
}

void main()
{
    pLight l1 = pLight(vec3(sec-3.0, 2.0*sin(sec), cos(sec)*3.0),
                       vec3(0.8), vec3(1.0, 0.0, 0.0), vec3(0.8, 0.0, 0.0));
    
    vec2 uv = (-iResolution.xy + 2.0*gl_FragCoord.xy)/iResolution.y;
    
    vec2 uv2 = gl_FragCoord.xy / iResolution.xy;
    int tx = int(uv2.x*512.0);
    
    vec3 target  = vec3(0.0, 0.0, 0.0);
	vec3 eye     = vec3(2.0, 2.0, 2.0);
    vec3 up      = vec3(0.0, 1.0, 0.0);
    
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
    
    if(c.y>0.0) {
        color = material(c, hit, color);
        color = color * phong(hit, eye, norm, l1, 2.0);
    }

    // LOGO
    float r = iResolution.x/iResolution.y;
    vec2 q = uv;
    q += vec2(0.7*r, 0.7);

    // left-down circle
    float d = sdCircle(q, 0.1);
    color = mix(color, vec3(1.0), smoothstep(3.0/iResolution.y, 0.0, d));
    
    // left down arc
    float ta = PI/2.0 * sec*2.0;// 3.14*(0.5+0.5*cos(iTime*0.52+2.0));
    float tb = PI/4.0 * (1.0-sin(sec)/2.0); //3.14*(0.5+0.5*cos(iTime*0.31+2.0));
    d = sdArc(q,vec2(sin(ta),cos(ta)),vec2(sin(tb),cos(tb)), 0.2, 0.02);
    color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));

	gl_FragColor = vec4(color, 1.0);
}