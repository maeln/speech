/* framework header */
#version 430
layout(location = 0) uniform vec4 iResolution;
layout(location = 1) uniform int iTime;

#define PI 3.1415
#define MAXSTEPS 128
#define MINDIST  0.0001
#define MAXDIST  20.0
#define saturate(x) (clamp(0.0, 1.0, x))
#define f1(x) (clamp(x, 0.0, 1.0))
#define rep(p, r) (mod(p - r*.5, r) - r*.5)

vec3 hi = vec3(0.0, 0.0, 0.0)/255.0;
vec3 lo = vec3(0.0, 0.0, 0.0)/255.0;

float sec = float(iTime) / 1000.0;

struct pLight {
    vec3 position;
    vec3 ambiant;
    vec3 diffuse;
    vec3 specular;
};

float random (in vec2 st) {
    return fract(sin(dot(st.xy,
                         vec2(12.9898,78.233)))*
        43758.5453123);
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

float opSmoothUnion( float d1, float d2, float k ) {
    float h = clamp( 0.5 + 0.5*(d2-d1)/k, 0.0, 1.0 );
    return mix( d2, d1, h ) - k*h*(1.0-h); 
}
    
float sdBoxFrame( vec3 p, vec3 b, float e )
{
  p = abs(p  )-b;
  vec3 q = abs(p+e)-e;
  return min(min(
      length(max(vec3(p.x,q.y,q.z),0.0))+min(max(p.x,max(q.y,q.z)),0.0),
      length(max(vec3(q.x,p.y,q.z),0.0))+min(max(q.x,max(p.y,q.z)),0.0)),
      length(max(vec3(q.x,q.y,p.z),0.0))+min(max(q.x,max(q.y,p.z)),0.0));
}

float sdSphere(vec3 p, float s)
{
    return length(p) - s;
}

float sdCapsule( vec3 p, vec3 a, vec3 b, float r )
{
  vec3 pa = p - a, ba = b - a;
  float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
  return length( pa - ba*h ) - r;
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

float sdSegment( in vec2 p, in vec2 a, in vec2 b )
{
    vec2 pa = p-a, ba = b-a;
    float h = clamp( dot(pa,ba)/dot(ba,ba), 0.0, 1.0 );
    return length( pa - ba*h );
}

float sdParallelogram( in vec2 p, float wi, float he, float sk )
{
    vec2 e = vec2(sk,he);
    p = (p.y<0.0)?-p:p;
    vec2  w = p - e; w.x -= clamp(w.x,-wi,wi);
    vec2  d = vec2(dot(w,w), -w.y);
    float s = p.x*e.y - p.y*e.x;
    p = (s<0.0)?-p:p;
    vec2  v = p - vec2(wi,0); v -= e*clamp(dot(v,e)/dot(e,e),-1.0,1.0);
    d = min( d, vec2(dot(v,v), wi*he-abs(s)));
    return sqrt(d.x)*sign(-d.y);
}

// Compute the position of particle {i} at the time {sec}.
vec3 calcPos(int i) {
    float fi = float(i);
    float dx = sin((fi/4.0)+sec) + mix(2.0, 0.0, f1(sec-4.0));
    float dy = sin(2.0*PI*(fi/10.0) + sec) - mix(0.0, sin(2.0*PI*(fi/10.0) + sec), f1(sec/1.5-5.0));
    float dz = cos(2.0*PI*(fi/10.0) + sec) - mix(0.0, cos(2.0*PI*(fi/10.0) + sec), f1(sec/1.5-5.0));
    
    vec3 dir = vec3(dx, dy, dz);
    vec3 p = dir;
    
    return p;
}

float sdTriangle(vec3 ray) {
    vec3 p1 = vec3(0.0);
    vec3 p2 = vec3(0.0, 0.0, 1.0);
    vec3 p3 = vec3(0.0, sqrt(1.0), 0.5);
    return min(min(sdCapsule(ray, p1, p2, 0.01), sdCapsule(ray, p2, p3, 0.01)), sdCapsule(ray, p3, p1, 0.01));
}


vec2 enter(vec3 ray) {
    float mi = 0.0;
    float de = MAXDIST;
    for(int i=1; i<50; ++i) {
        float fi = float(i);
        float zr = 2. - random(vec2(fi, 5.0)) * 4.0;
        float yr = 2. - random(vec2(fi, 2.0)) * 4.0;
        float xr = -10.0 + mix((fi/50.0*10.0), 0.0, saturate(sec/20.0)) + mod(sec*2.0+(random(vec2(fi, 1.0))*20.0), 20.0);
        vec3 q = ray - vec3(xr, yr, zr);
        float rx = 2. - random(vec2(fi, 5.0)) * 4.0;
        q = (rotationMatrix(vec3(rx, yr, zr), sec) * vec4(q, 1.0)).xyz;
        float di = sdTriangle(q);
        if(de > di) {
            de = di;
            mi = fi;
        }
    }
    return vec2(de, mi);
}

vec2 dancingSphere(vec3 ray) {
    float mi = 0.0;
    float de = MAXDIST;
    for(int i=0; i<10; ++i) {
        float di = sdSphere(ray-calcPos(i), 0.1);
        if(de > di) {
            de = di;
            mi = float(i)+1.0;
        }
    }
    
    return vec2(de, mi);
}

float middleBox(vec3 ray) {
    float zr = 2. - random(vec2(7.0, 5.0)) * 4.0;
    float yr = 2. - random(vec2(2.0, 2.0)) * 4.0;
    float rx = 2. - random(vec2(16.0, 5.0)) * 4.0;
    vec3 q = ray;
    q = (rotationMatrix(vec3(rx, yr, zr), sec) * vec4(ray, 1.0)).xyz;
    return sdBoxFrame(q, vec3(0.2, 0.2, 0.2) * vec3(f1(sec/1.5 - 5.0/1.5)), 0.025);
}

// Draw all the particles
vec3 scene(vec3 ray)
{
    vec2 init = enter(ray);
    vec2 spheres = dancingSphere(ray);
    float cube = middleBox(ray);
    
    float rde = opSmoothUnion(spheres.x, cube, 0.1);
    
    float kmi = init.y;
    float kde = init.x;
    float koi = 0.0;
    
    if(spheres.x < init.x) {
        kmi = spheres.y;
        kde = spheres.x;
        koi = 1.0;
    }
    
    if(sec > 5.0 && cube < kde) {
        kde = cube;
        koi = 1.0;
    }
    
    
    return vec3(kde, kmi, koi);
}

vec3 normal(vec3 pos) {
    vec2 eps = vec2(0.0, MINDIST);
	return normalize(vec3(
    scene(pos + eps.yxx).x - scene(pos - eps.yxx).x,
    scene(pos + eps.xyx).x - scene(pos - eps.xyx).x,
    scene(pos + eps.xxy).x - scene(pos - eps.xxy).x));
}

vec4 raymarch(vec3 from, vec3 direction)
{
    float t = 1.0*MINDIST;
    int i = 0;
    vec3 march = vec3(MAXDIST, -1.0, -1.0);
    for(int steps=0; steps<MAXSTEPS; ++steps)
    {
        ++i;
        march = scene(from + t * direction);
        if(march.x < MINDIST || t >= MAXDIST) break;
        t += march.x;
    }
    return vec4(t, t > MAXDIST ? -1.0 : march.y, march.z, float(i));
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

void main()
{
    pLight l1 = pLight(vec3(sin(sec/3.0), sin(sec/4.0), cos(sec/6.0)),
                       vec3(0.0), vec3(0.0), vec3(1.8));
    
    vec2 uv = (2.0*gl_FragCoord.xy-iResolution.xy)/iResolution.y;
    
    vec3 target  = vec3(0.0, 0.0, 0.0);
    vec3 eye     = vec3(2.0, 0.0, 0.0);
    vec3 up      = vec3(0.0, 1.0, 0.0);
    
    vec3 eyeDir   = normalize(target - eye);
    vec3 eyeRight = normalize(cross(up, eye));
    vec3 eyeUp    = normalize(cross(eye, eyeRight));
    
    vec3 rayDir = normalize(eyeRight * uv.x + eyeUp * uv.y + eyeDir);
       
    vec3 skyCol = mix(lo, hi, gl_FragCoord.y/iResolution.y);
    vec3 color = skyCol;
    
    vec4 march = raymarch(eye, rayDir);
    color = vec3(0.3, 0.0, 1.0) * march.w / float(MAXSTEPS) * 4.0;
    if(march.y>0.0) {
        color = vec3(1.0);
        if(march.z>0.0) {
            vec3 hit = eye+march.x*rayDir;
            vec3 norm = normal(hit);
            color *= phong(hit, eye, norm, l1, 16.0);
        }
    }
    
    
    // LOGO
    // left-down circle
    float d = sdCircle(uv + vec2(0.6, 0.6), 0.05);
    color = mix(color, vec3(1.0), smoothstep(3.0/iResolution.y, 0.0, d));
    
    // left down arc
    float ta = PI/2.0 * sec*2.0;// 3.14*(0.5+0.5*cos(sec*0.52+2.0));
    float tb = PI/4.0 * (1.0-sin(sec)/2.0); //3.14*(0.5+0.5*cos(sec*0.31+2.0));
    d = sdArc(uv + vec2(0.6, 0.6),vec2(sin(ta),cos(ta)),vec2(sin(tb),cos(tb)), 0.1, 0.005);
    color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));
    
    // line top left
    d = sdSegment(uv, vec2(-2.0, 0.2), vec2(0.25, 0.2));
    color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));
    
    // line bottom right
    d = sdSegment(uv, vec2(-0.25, -0.2), vec2(2.0, -0.2));
    color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));
    
    // top roller
    for(float i=0.0; i<48.0; i+=1.0) {
      d = sdParallelogram(uv+vec2(mix(-4.0, 4.0, i/48.0)-mod(sec, 1.0), -0.275), 0.05, 0.05, 0.03);
      color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));
    }
    
    // bottom roller
    for(float i=0.0; i<48.0; i+=1.0) {
      d = sdParallelogram(uv+vec2(mix(-4.0, 4.0, i/48.0)+mod(sec, 1.0), 0.275), 0.05, 0.05, 0.03);
      color = mix(color, vec3(1.0), smoothstep(4.0/iResolution.y, 0.0, d));
    }
    
    gl_FragColor = vec4(color, 1.0);
}