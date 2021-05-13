/* File generated with Shader Minifier 1.1.6
 * http://www.ctrl-alt-test.fr
 */
#ifndef SHADER_CODE_H_
# define SHADER_CODE_H_
# define VAR_FRAMEBUFFERTEX "l"
# define VAR_IRESOLUTION "v"

const char *postprocess_frag =
 "#version 430\n"
 "layout(location=0)uniform vec4 v;"
 "layout(binding=0)uniform sampler2D l;"
 "void main()"
 "{"
   "vec4 v=texelFetch(l,ivec2(gl_FragCoord.xy),0);"
   "vec3 g=v.xyz/v.w;"
   "gl_FragColor=vec4(g,1);"
 "}";

#endif // SHADER_CODE_H_
