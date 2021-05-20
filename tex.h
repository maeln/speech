/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef TEX_H_
# define TEX_H_
# define VAR_IRESOLUTION "i"
# define VAR_ITIME "m"
# define VAR_TEXSAMPLER "l"

const char *tex_frag =
 "#version 430\n"
 "layout(location=0)uniform vec4 i;"
 "layout(location=1)uniform int m;"
 "layout(binding=0)uniform sampler2D l;"
 "void main()"
 "{"
   "vec4 m=texture(l,gl_FragCoord.xy/i.xy);"
   "gl_FragColor=m;"
 "}";

#endif // TEX_H_
