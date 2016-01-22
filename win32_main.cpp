/** Copyright 2016 Ryan Caso
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
**/

// This code file was made with Visual Studio 2015
// See the accompanying .props file for includes and libs
// or use the associated Template to create a project with
// this file but with your project name as the windows titles
// and class names.  Both the .prop and Template include and
// lib settings will probably need to be altered to fit your
// environment.  The Template should have a readme explaining
// some of the reasoning and a reliable setup for visual studio.

#include <Windows.h>
#include "GL\glew.h"
#include <GL\wglew.h> // wglChoosePixelFormatARB and wglCreateContextAttribsARB are in here

HGLRC hGLRenderContext;

bool bIsRunning;
bool bCanDraw;
bool isOpenGL45;

GLuint gShaderProg;
GLuint gVAO;
GLuint gVBO;

LRESULT CALLBACK MainWindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_QUIT:
		{
			bIsRunning = false;
		}break;
		default:
		{
			
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

GLuint CompileShader(const wchar_t* shaderFile, GLenum shaderType)
{
	GLuint shader = glCreateShader(shaderType);

	char dataBuffer[1024] = { 0 };
	DWORD numBytesToRead = 1024;
	DWORD numBytesRead = 0;
	HANDLE hFile = CreateFileW(shaderFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	HRESULT hError = GetLastError();
	ReadFile(hFile, &dataBuffer, numBytesToRead, &numBytesRead, NULL);

	const char* shaderSource = dataBuffer;
	glShaderSource(shader, 1, &shaderSource, 0);
	glCompileShader(shader);

	GLint compileStatus;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compileStatus);
	if (compileStatus == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		char* infoLog = new char[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
		size_t numBytesConverted = 0;
		wchar_t outputString[512];
		mbstowcs_s(&numBytesConverted, outputString, (size_t)0, (const char*)infoLog, (size_t)512);
		OutputDebugString(outputString);
		delete[] infoLog;
		glDeleteShader(shader);
		return 0;
	}

	CloseHandle(hFile);
	hFile = 0;

	return shader;
}

bool CompileAndLinkShaders()
{
	bool result = true;
	
    // These paths are made relative to a standard project setup in Visual Studio 2015, you will have to alter these
	GLuint vertShader = CompileShader(L"..\\$(safeprojectname)\\Shaders\\DefVertShader.glsl", GL_VERTEX_SHADER);
	GLuint fragShader = CompileShader(L"..\\$(safeprojectname)\\Shaders\\DefFragShader.glsl", GL_FRAGMENT_SHADER);

	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertShader);
	glAttachShader(shaderProgram, fragShader);
	glLinkProgram(shaderProgram);
	glDetachShader(shaderProgram, vertShader);
	glDetachShader(shaderProgram, fragShader);

	gShaderProg = shaderProgram;

	return result;
}

void BindBuffers()
{
	glGenVertexArrays(1, &gVAO);
	glBindVertexArray(gVAO);

	glGenBuffers(1, &gVBO);
	glBindBuffer(GL_ARRAY_BUFFER, gVBO);
    
    // This creates a square with just vertices.  For larger drawings I recommend looking into
    // indexed drawing with index buffers not just vertex buffers.
	GLfloat vertexData[] = {
		-0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, 0.5f, 0.0f,

		0.5f, 0.5f, 0.0f,
		-0.5f, -0.5f, 0.0f,
		0.5f, -0.5f, 0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

	GLint attrib = glGetAttribLocation(gShaderProg, "vert");
	if (-1 == attrib)
	{
		OutputDebugString(L"Attribute not found!");
	}
	glEnableVertexAttribArray(attrib);
	glVertexAttribPointer(attrib, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void MainMessagePump(HDC hDC)
{
	MSG message;
	if (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		switch (message.message)
		{
			case WM_KEYDOWN:
			case WM_KEYUP:
			{
                // currently these key presses and releases get spammed.
                // bits 30 and 31 can be used to check if the key was already
                // down and if it is down.  Search MSDN for MSG structure and
                // WM_KEYDOWN or WM_KEYUP for more information on handling these. 
				if (message.wParam == VK_ESCAPE)
				{
					bIsRunning = 0;
				}
				if (message.wParam == VK_SPACE)
				{
                    
				}
				if (message.wParam == 65)
				{
					// this is the A key
                }
                
                // need to know what code is being send for a key that doesn't
                // have a VK_ defined?  Uncomment this, compile and run your app,
                // press it and look at visual studios output window for the answer.
				/*wchar_t outText[32];
				wsprintf(outText, L"Key: %i, ", message.wParam);
				if (outText) {
					OutputDebugString(outText);
				}*/
			}break;
			default:
			{
				TranslateMessage(&message);
				DispatchMessage(&message);
			}
		}
	}
	else
	{
		// Update and Render
		if (bCanDraw)
		{
			glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUseProgram(gShaderProg);
			glBindVertexArray(gVAO);
			glDrawArrays(GL_TRIANGLES, 0, 6);
			glBindVertexArray(0);
			glUseProgram(0);

			// swap buffers (aka present)
			SwapBuffers(hDC);
		}
	}
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
{
	HDC hDC;
	isOpenGL45 = false;

    WNDCLASS windowClass = {};
	windowClass.style = CS_VREDRAW | CS_HREDRAW | CS_OWNDC;
	windowClass.lpfnWndProc = MainWindowProc;
	windowClass.hInstance = hInstance;
	windowClass.hIcon = LoadIcon(0, IDI_APPLICATION);
	windowClass.hCursor = LoadCursor(0, IDC_CROSS);
	windowClass.hbrBackground = (HBRUSH)(COLOR_BACKGROUND + 1);
	windowClass.lpszClassName = L"$(safeporjectname)_MainWindowClass";
    
    RegisterClass(&windowClass);
	
	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),
		1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,    //Flags
		PFD_TYPE_RGBA,            //The kind of framebuffer. RGBA or palette.
		32,                        //Colordepth of the framebuffer.
		24, 0, 0, 0, 0, 0,
		0,
		0,
		0,
		0, 0, 0, 0,
		24,                        //Number of bits for the depthbuffer
		8,                        //Number of bits for the stencilbuffer
		0,                        //Number of Aux buffers in the framebuffer.
		PFD_MAIN_PLANE,
		0,
		0, 0, 0
	};
    
    HWND hWnd = CreateWindow(L"$(safeporjectname)_MainWindowClass", L"$(safeporjectname)", WS_POPUP , CW_USEDEFAULT, CW_USEDEFAULT, 960, 540, 0, 0, hInstance, 0);
	if (hWnd)
	{
		int err = GetLastError();
		if (err) {
			MessageBox(NULL, L"Error creating window.", L"Fatal Error!", MB_OK);
			return 0;
		}

		int pixelFormat;
		hDC = GetDC(hWnd);
		err = GetLastError();
		if (err) {
			MessageBox(NULL, L"Error getting Device Context, make sure WNDCLASS.style has CS_OWNDC set.", L"Fatal Error!", MB_OK);
			return 0;
		}

		pixelFormat = ChoosePixelFormat(hDC, &pfd);
		err = GetLastError();
		if (err) {
			MessageBox(NULL, L"Error requesting pixel format.", L"Fatal Error!", MB_OK);
			return 0;
		}
		SetPixelFormat(hDC, pixelFormat, &pfd);

		hGLRenderContext = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hGLRenderContext);

		GLenum errGlew = glewInit();

		if (GLEW_VERSION_4_5)
		{
			OutputDebugString(L"OpenGL 4.5 is available.");
			isOpenGL45 = true;
		}

		wglMakeCurrent(hDC, NULL);
		wglDeleteContext(hGLRenderContext);
		hGLRenderContext = 0;

		ReleaseDC(hWnd, hDC);
		hDC = 0;

		DestroyWindow(hWnd);
		hWnd = 0;
	}

	if (!isOpenGL45)
	{
		OutputDebugString(L"OpenGL 4.5 is not available, closing app.");
		return 0;
	}

	hWnd = CreateWindow(L"$(safeporjectname)_MainWindowClass", L"$(safeporjectname)", WS_POPUP | WS_VISIBLE, 100, 100, 960, 540, 0, 0, hInstance, 0);
	hDC = GetDC(hWnd);
	int attributeListInt[19];
	int pixelFormat[1];
	UINT formatCount;
	int attributeList[5];

	// Support for OpenGL rendering.
	attributeListInt[0] = WGL_SUPPORT_OPENGL_ARB;
	attributeListInt[1] = TRUE;

	// Support for rendering to a window.
	attributeListInt[2] = WGL_DRAW_TO_WINDOW_ARB;
	attributeListInt[3] = TRUE;

	// Support for hardware acceleration.
	attributeListInt[4] = WGL_ACCELERATION_ARB;
	attributeListInt[5] = WGL_FULL_ACCELERATION_ARB;

	// Support for 32bit color.
	attributeListInt[6] = WGL_COLOR_BITS_ARB;
	attributeListInt[7] = 32;

	// Support for 24 bit depth buffer.
	attributeListInt[8] = WGL_DEPTH_BITS_ARB;
	attributeListInt[9] = 24;

	// Support for double buffer.
	attributeListInt[10] = WGL_DOUBLE_BUFFER_ARB;
	attributeListInt[11] = TRUE;

	// Support for swapping front and back buffer.
	attributeListInt[12] = WGL_SWAP_METHOD_ARB;
	attributeListInt[13] = WGL_SWAP_EXCHANGE_ARB;

	// Support for the RGBA pixel type.
	attributeListInt[14] = WGL_PIXEL_TYPE_ARB;
	attributeListInt[15] = WGL_TYPE_RGBA_ARB;

	// Support for a 8 bit stencil buffer.
	attributeListInt[16] = WGL_STENCIL_BITS_ARB;
	attributeListInt[17] = 8;

	// Null terminate the attribute list.
	attributeListInt[18] = 0;

	wglChoosePixelFormatARB(hDC, attributeListInt, NULL, 1, pixelFormat, &formatCount);
	SetPixelFormat(hDC, pixelFormat[0], &pfd);

	attributeList[0] = WGL_CONTEXT_MAJOR_VERSION_ARB;
	attributeList[1] = 4;
	attributeList[2] = WGL_CONTEXT_MINOR_VERSION_ARB;
	attributeList[3] = 5;
	attributeList[4] = 0;

	hGLRenderContext = wglCreateContextAttribsARB(hDC, 0, attributeList);
	wglMakeCurrent(hDC, hGLRenderContext);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glFrontFace(GL_CCW);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	char* vendorString = (char*)glGetString(GL_VENDOR);
	char* rendererString = (char*)glGetString(GL_RENDERER);

    bIsRunning = true;
	bCanDraw = false;

	if (CompileAndLinkShaders())
	{
		BindBuffers();
		bCanDraw = true;
	}
	else
	{
		bIsRunning = false;
	}
    
    if(hWnd)
    {
        while(bIsRunning)
        {
            MainMessagePump(hDC);
        }
    }

	wglMakeCurrent(hDC, NULL);
	wglDeleteContext(hGLRenderContext);
    
    return 0;
}