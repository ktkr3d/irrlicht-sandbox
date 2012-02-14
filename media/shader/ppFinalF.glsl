uniform sampler2D tex;


uniform sampler2D tex2;
varying vec2 tcoords;

void main()
{
    vec4 col = texture2D(tex,tcoords*vec2(1.0,-1.0));
//Tonemap stage and gamma
    gl_FragColor =pow(vec4(col.rgba*col.a),2.2);
}
