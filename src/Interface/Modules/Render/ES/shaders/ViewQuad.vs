attribute vec3 aPos;
attribute vec2 aUV0;

varying vec2 fTexCoord;

void main()
{
  fTexCoord = aUV0;
  gl_Position = vec4(aPos, 1.0);
}
