uniform sampler2D uTX0;

varying vec2 fTexCoord;

void main()
{
  gl_FragColor = texture2D(uTX0, fTexCoord);
}