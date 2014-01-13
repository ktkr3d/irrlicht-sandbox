#ifndef CLASS_AVI_CREATOR_H_INCLUCED
#define CLASS_AVI_CREATOR_H_INCLUCED

//#include <windows.h>
//#include <vfw.h>
#define LONG long
#define DWORD long

#include <GL/glew.h>
#include <GL/glut.h>

class CAviCreator
{
public:
	CAviCreator(LONG width, LONG height,
			const char *file, LONG scale, LONG rate);
	~CAviCreator();
	bool CaptureWindow();

private:
	LONG m_Width;
	LONG m_Height;
	DWORD m_Size;
	LONG m_Scale;
	LONG m_Rate;
	LONG m_FrameCount;
	PAVIFILE m_pAviFile;
	PAVISTREAM m_pCompressed;
	PAVISTREAM m_pAviStream;
	COMPVARS m_cv;
	GLubyte* m_Pixels;
	GLuint m_PboID[2];

	bool Open(const char* file);
	void Close();
	void CreatePBO(GLuint *id, GLuint size);
};

#endif // CLASS_AVI_CREATOR_H_INCLUCED
