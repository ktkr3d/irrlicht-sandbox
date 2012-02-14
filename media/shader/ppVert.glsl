

varying vec2 tcoords;

void main()
	{
        tcoords= gl_MultiTexCoord0.xy;
        vec2 scrn=gl_MultiTexCoord1.xy;
        scrn.y=1.0-scrn.y;
        gl_Position = vec4(scrn*2.0-1.0, 0.0, 1.0);
	}
