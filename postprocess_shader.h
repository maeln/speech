/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef POSTPROCESS_SHADER_H_
# define POSTPROCESS_SHADER_H_
# define VAR_FRAMEBUFFERTEX "f"
# define VAR_IRESOLUTION "v"
# define VAR_ITIME "i"

const char *postprocess_frag =
 "#version 430\n"
 "layout(location=0)uniform vec4 v;"
 "layout(location=1)uniform int i;"
 "layout(binding=0)uniform sampler2D f;\n"
 "#define ITER_DIST 16\n"
 "#define saturate(x)(clamp((x),0.0,1.0))\n"
 "float t(in vec2 v)"
 "{"
   "return fract(sin(dot(v.xy,vec2(12.9898,78.233)))*43758.5);"
 "}"
 "float n(in vec2 v)"
 "{"
   "vec2 i=floor(v),f=fract(v);"
   "float x=t(i),y=t(i+vec2(1.,0.)),g=t(i+vec2(0.,1.)),n=t(i+vec2(1.,1.));"
   "vec2 s=f*f*(3.-2.*f);"
   "return mix(x,y,s.x)+(g-x)*s.y*(1.-s.x)+(n-y)*s.x*s.y;"
 "}"
 "float n(in vec2 v,float f)"
 "{"
   "float i=0.,t=f;"
   "for(int r=0;r<4;r++)"
     "i+=t*n(v),v*=4.3,t*=.5;"
   "return i;"
 "}"
 "vec3 e(float v)"
 "{"
   "return v=v*3.-1.5,saturate(vec3(-v,1.-abs(v),v));"
 "}"
 "vec2 e(vec2 v,float f)"
 "{"
   "float s=(.1-n(vec2(v.y*4.,float(i)/4000.),f))/2.;"
   "return vec2(v.x+s,v.y);"
 "}"
 "vec3 r(vec2 v)"
 "{"
   "const float i=1./(float(ITER_DIST)-1.);"
   "float r=i;"
   "vec3 s=vec3(0.),t=vec3(0.);"
   "for(int x=0;x<ITER_DIST;++x)"
     "{"
       "vec3 n=e(r);"
       "t+=n;"
       "s+=n*texture(f,e(v,.1+float(x)/float(ITER_DIST)/32.)).xyz;"
       "r+=i;"
     "}"
   "s/=t;"
   "return s;"
 "}"
 "void main()"
 "{"
   "vec3 s=r(gl_FragCoord.xy/v.xy),t=texture(f,gl_FragCoord.xy/v.xy).xyz;"
   "float x=float(i)/1000.,y=(1.+sin(5.*sin(2.*sin(4.*sin(x))))*sin(8.*x)*sin(x/3.))/2.;"
   "vec3 n=t*y+s*(1.-y);"
   "gl_FragColor=vec4(n,1);"
 "}";

#endif // POSTPROCESS_SHADER_H_
