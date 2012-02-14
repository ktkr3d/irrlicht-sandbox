uniform sampler2D tex;


uniform sampler2D tex2;
varying vec2 tcoords;

void main()
{
    vec4 col = texture2D(tex,tcoords*vec2(1.0,-1.0))+texture2D(tex2,tcoords);
    //col*=col.a;
    gl_FragColor =col;
}
