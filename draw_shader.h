/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef DRAW_SHADER_H_
# define DRAW_SHADER_H_
# define VAR_IFRAME "v"
# define VAR_IRESOLUTION "m"

const char *draw_frag =
 "#version 430\n"
 "layout(location=0)uniform vec4 m;"
 "layout(location=1)uniform int v;"
 "const float a=acos(-1.);"
 "mat2 t(float v)"
 "{"
   "float f=cos(v),n=sin(v);"
   "return mat2(f,-n,n,f);"
 "}"
 "float t(vec3 v,float f)"
 "{"
   "return(dot(abs(v),vec3(1))-f)/sqrt(3.);"
 "}"
 "float n(vec3 v)"
 "{"
   "float f=t(v,1.5);"
   "v.y=abs(v.y)-.15;"
   "f=max(f,-t(v,1.5));"
   "return f;"
 "}"
 "vec3 n(vec3 v,float f)"
 "{"
   "return mod(v-f,f+f)-f;"
 "}"
 "float f(vec3 v)"
 "{"
   "float f=1e+09,a=4.;"
   "for(int r=0;r<3;++r)"
     "f=min(f,n(n(v+vec3(a,0,a),a))),f=min(f,n(n(v+vec3(0,a,0),a))),v=v.yzx;"
   "return f;"
 "}"
 "float r;"
 "float f()"
 "{"
   "float v=fract(r++*.1031);"
   "v+=v*(v+19.19)*3.;"
   "return fract((v+v)*v);"
 "}"
 "vec2 n()"
 "{"
   "return vec2(f(),f());"
 "}"
 "void main()"
 "{"
   "r=float((v*73856093^int(gl_FragCoord.x)*19349663^int(gl_FragCoord.y)*83492791)%38069);"
   "vec2 y=(gl_FragCoord.xy+n()-.5)/m.xy-.5;"
   "y.x*=m.z;"
   "y*=4.+length(y);"
   "vec3 d=vec3(0,0,-10),z=normalize(vec3(y,1));"
   "vec2 c=(n()-.5)*t(a*.25);"
   "const float l=.5,s=17.;"
   "d.xy+=c*l;"
   "z.xy-=c*l/s;"
   "d.yz*=t(atan(1.,sqrt(2.)));"
   "z.yz*=t(atan(1.,sqrt(2.)));"
   "d.xz*=t(a*.75);"
   "z.xz*=t(a*.75);"
   "vec3 i=vec3(.2*length(y),.1,.25);"
   "const float g=.001;"
   "float x=0.,p=0.;"
   "for(int o=0;o<200;++o)"
     "{"
       "p=f(d+z*x);"
       "if(abs(p)<g)"
         "break;"
       "x+=p;"
     "}"
   "if(abs(p)<g)"
     "{"
       "vec3 o=d+z*x;"
       "vec2 e=vec2(g,0);"
       "vec3 u=normalize(vec3(f(o+e.xyy),f(o+e.yxy),f(o+e.yyx))-p);"
       "float b=dot(u,normalize(vec3(1,2,3)))*.35+.65,F=min(1.,pow(.9,x-20.));"
       "i=mix(i,vec3(b),F);"
     "}"
   "gl_FragColor=vec4(i,1.);"
 "}";

#endif // DRAW_SHADER_H_
