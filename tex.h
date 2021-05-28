/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef TEX_H_
# define TEX_H_
# define VAR_IRESOLUTION "i"
# define VAR_ITIME "v"
# define VAR_SDFTEX "l"

const char *tex_frag =
 "#version 430\n"
 "layout(location=0)uniform vec4 i;"
 "layout(location=1)uniform int v;"
 "uniform sampler2D l;"
 "void main()"
 "{"
   "vec4 v=texelFetch(l,ivec2(gl_FragCoord.xy),0);"
   "v.xyz=vec3(1.,1.,1.);"
   "gl_FragColor=v;"
 "}";

#endif // TEX_H_
