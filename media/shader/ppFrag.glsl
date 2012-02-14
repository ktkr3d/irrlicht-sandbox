uniform sampler2D tex;

varying vec2 tcoords;

void main()
	{
	    vec4 col = texture2D(tex,tcoords);
	    //col*=col.a;
		gl_FragColor =col*col.a;
	}
