#pragma once

#include "gl.h"

namespace ImApp
{

	struct Config
	{
		Config() : mWidth(1280), mHeight(720), mFullscreen(false)
		{

		}
		int mWidth;
		int mHeight;
		bool mFullscreen;
	};

	struct ImApp
	{
		ImApp() : mInitExtensionsDone(false), mExtensionsPresent(false), mDone(true)
		{
			mInstance = this;
		}
		~ImApp()
		{
			mInstance = NULL;
		}
		static ImApp* Instance() { return mInstance; }
		int Init(const Config& config = Config())
		{
			mConfig = config;
			wininfo = WININFO{ 0,0,0,0,
			{ 'c','X','d',0 }
			};
			WININFO     *info = &wininfo;

			info->hInstance = GetModuleHandle(0);

			static DEVMODEA screenSettings = { { 0 },
#if _MSC_VER < 1400
				0,0,148,0,0x001c0000,{ 0 },0,0,0,0,0,0,0,0,0,{ 0 },0,32,config.mWidth,config.mHeight,0,0,      // Visual C++ 6.0
#else
				0,0,156,0,0x001c0000,{ 0 },0,0,0,0,0,{ 0 },0,32,config.mWidth,config.mHeight,{ 0 }, 0,           // Visuatl Studio 2005
#endif
#if(WINVER >= 0x0400)
				0,0,0,0,0,0,
#if (WINVER >= 0x0500) || (_WIN32_WINNT >= 0x0400)
				0,0
#endif
#endif
			};
			if (config.mFullscreen)
			{
				if (config.mFullscreen && ChangeDisplaySettingsA(&screenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
					return 0;
			}
#ifdef IMGUI_API
			ImGui::CreateContext();
#endif
			if (!WindowInit(info))
			{
				WindowEnd(info);
				return 0;
			}
			if (!InitExtension())
				return 0;

#ifdef IMGUI_API
			if (!ImGui_Init())
				return 0;

#endif
			mDone = false;
			return 1;
		}

		void LoadBanks(int bankCount, const char **bankPaths);
		void PlayEvent(const char *eventName);

		void NewFrame()
		{
			MSG msg;
			while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);

				if (msg.message == WM_QUIT)
					mDone = true;
			}

#ifdef IMGUI_API
			ImGui_NewFrame();
#endif
		}

		void EndFrame()
		{
#ifdef IMGUI_API
			ImGui::Render();
#endif
			SwapBuffers(wininfo.hDC);
		}
		void Finish()
		{
#ifdef IMGUI_API
			ImGui_Shutdown();
#endif
		}
		bool Done()
		{
			return mDone;
		}
	protected:
		static ImApp *mInstance;
		Config mConfig;

		bool mInitExtensionsDone;
		bool mExtensionsPresent;
		bool mDone;
		typedef struct
		{
			//---------------
			HINSTANCE   hInstance;
			HDC         hDC;
			HGLRC       hRC;
			HWND        hWnd;
			char        wndclass[4];	// window class and title :)
										//---------------
		}WININFO;

		WININFO wininfo;

		static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
#ifdef IMGUI_API
			if (ImGui_WndProcHandler(hWnd, msg, wParam, lParam))
				return true;
#endif
			switch (msg)
			{
			case WM_SIZE:
			{
#ifdef IMGUI_API
				ImGuiIO& io = ImGui::GetIO();

				int w, h;
				int display_w, display_h;
				//glfwGetWindowSize(g_Window, &w, &h);
				//glfwGetFramebufferSize(g_Window, &display_w, &display_h);
				w = LOWORD(lParam); // width of client area
				h = HIWORD(lParam); // height of client area
				io.DisplaySize = ImVec2((float)w, (float)h);
				display_w = w;
				display_h = h;
				io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
#endif
			}
				return 0;
			case WM_SYSCOMMAND:
				if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
					return 0;
				break;
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;
			}
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}


		void WindowEnd(WININFO *info)
		{
			if (info->hRC)
			{
				wglMakeCurrent(0, 0);
				wglDeleteContext(info->hRC);
			}

			if (info->hDC) ReleaseDC(info->hWnd, info->hDC);
			if (info->hWnd) DestroyWindow(info->hWnd);

			UnregisterClassA(info->wndclass, info->hInstance);

			if (mConfig.mFullscreen)
			{
				ChangeDisplaySettings(0, 0);
				while (ShowCursor(1)<0); // show cursor
			}
		}

		int WindowInit(WININFO *info)
		{
			unsigned int	PixelFormat;
			DWORD			dwExStyle, dwStyle;
			DEVMODE			dmScreenSettings;
			RECT			rec;

			WNDCLASSEXA WndClsEx;

			// Create the application window
			WndClsEx.cbSize = sizeof(WNDCLASSEX);
			WndClsEx.style = CS_HREDRAW | CS_VREDRAW;
			WndClsEx.lpfnWndProc = WndProc;
			WndClsEx.cbClsExtra = 0;
			WndClsEx.cbWndExtra = 0;
			WndClsEx.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			WndClsEx.hCursor = LoadCursor(NULL, IDC_ARROW);
			WndClsEx.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			WndClsEx.lpszMenuName = NULL;
			WndClsEx.lpszClassName = info->wndclass;
			WndClsEx.hInstance = info->hInstance;
			WndClsEx.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

			if (!RegisterClassExA(&WndClsEx))
				return 0;

			if (mConfig.mFullscreen)
			{
				dmScreenSettings.dmSize = sizeof(DEVMODE);
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmPelsWidth = mConfig.mWidth;
				dmScreenSettings.dmPelsHeight = mConfig.mHeight;

				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
					return(0);

				dwExStyle = WS_EX_APPWINDOW;
				dwStyle = WS_VISIBLE | WS_POPUP;

				//while (ShowCursor(0) >= 0);	// hide cursor
			}
			else
			{
				dwExStyle = 0;
				dwStyle = WS_VISIBLE | WS_CAPTION | WS_SYSMENU | WS_MAXIMIZEBOX | WS_OVERLAPPED;
				dwStyle = WS_VISIBLE | WS_OVERLAPPEDWINDOW | WS_POPUP;
			}

			rec.left = 0;
			rec.top = 0;
			rec.right = mConfig.mWidth;
			rec.bottom = mConfig.mHeight;

			AdjustWindowRect(&rec, dwStyle, 0);

			info->hWnd = CreateWindowExA(dwExStyle, WndClsEx.lpszClassName, "", dwStyle| WS_MAXIMIZE,
				(GetSystemMetrics(SM_CXSCREEN) - rec.right + rec.left) >> 1,
				(GetSystemMetrics(SM_CYSCREEN) - rec.bottom + rec.top) >> 1,
				rec.right - rec.left, rec.bottom - rec.top, 0, 0, info->hInstance, 0);

			if (!info->hWnd)
				return(0);

			if (!(info->hDC = GetDC(info->hWnd)))
				return(0);

			static PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				32,
				0, 0, 0, 0, 0, 0, 8, 0,
				0, 0, 0, 0, 0,  // accum
				32,             // zbuffer
				0,              // stencil!
				0,              // aux
				PFD_MAIN_PLANE,
				0, 0, 0, 0
			};

			if (!(PixelFormat = ChoosePixelFormat(info->hDC, &pfd)))
				return(0);

			if (!SetPixelFormat(info->hDC, PixelFormat, &pfd))
				return(0);

			if (!(info->hRC = wglCreateContext(info->hDC)))
				return(0);

			if (!wglMakeCurrent(info->hDC, info->hRC))
				return(0);

			return(1);
		}


		bool loadExtension(const char *extensionName, void **functionPtr)
		{
#if defined(_WIN32)
			*functionPtr = glGetProcAddress(extensionName);
#else
			*functionPtr = (void *)glGetProcAddress((const GLubyte *)extensionName);
#endif
			return (*functionPtr != NULL);
		}
#define LE(x) mExtensionsPresent &= loadExtension(#x, (void**)&x);

		bool InitExtension()
		{
			if (mInitExtensionsDone)
				return true;

			mExtensionsPresent = true;

			LE(glUniform1i); //GLint location, GLint v0);
			LE(glUniformMatrix3fv) // GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
#ifdef _MSC_VER
				LE(glActiveTexture); //GLenum texture);
#endif
			LE(glBindFramebuffer); //GLenum target, TextureID framebuffer);
			LE(glDeleteFramebuffers); //GLsizei n, const TextureID* framebuffers);
			LE(glDeleteRenderbuffers); //GLsizei n, const TextureID* renderbuffers);
			LE(glFramebufferTexture2D); //GLenum target, GLenum attachment, GLenum textarget, TextureID texture, GLint level);
			LE(glFramebufferRenderbuffer); //GLenum target, GLenum attachment, GLenum renderbuffertarget, TextureID renderbuffer);
			LE(glRenderbufferStorage); //GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
			LE(glGenFramebuffers); //GLsizei n, TextureID* framebuffers);
			LE(glGenRenderbuffers); //GLsizei n, TextureID* renderbuffers);
			LE(glBindRenderbuffer); //GLenum target, TextureID renderbuffer);
			LE(glCheckFramebufferStatus); //GLenum target);
			LE(glGenerateMipmap); //GLenum target);
			LE(glBufferData); //GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
			LE(glUseProgram); //TextureID program);
			LE(glGetUniformLocation); //TextureID program, const GLchar* name);
			LE(glGetAttribLocation); //TextureID program, const GLchar* name);
			LE(glDeleteBuffers); //GLsizei n, const TextureID* buffers);
			LE(glDeleteVertexArrays); //GLsizei n, const TextureID* arrays);
			LE(glEnableVertexAttribArray); //TextureID);
			LE(glVertexAttribPointer); //TextureID index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* pointer);
			LE(glGenBuffers); //GLsizei n, TextureID* buffers);
			LE(glBindBuffer); //GLenum target, TextureID buffer);
			LE(glCreateShader); //GLenum type);
			LE(glShaderSource); //TextureID shader, GLsizei count, const GLchar** strings, const GLint* lengths);
			LE(glCompileShader); //TextureID shader);
			LE(glCreateProgram); //void);
			LE(glAttachShader); //TextureID program, TextureID shader);
			LE(glDeleteProgram); //TextureID program);
			LE(glDeleteShader); //TextureID shader);
			LE(glDisableVertexAttribArray); //TextureID);
			LE(glBindAttribLocation); //TextureID program, TextureID index, const GLchar* name);
			LE(glVertexAttribDivisor); //TextureID index, TextureID divisor);
			LE(glUniformMatrix4fv); //GLint location, GLsizei count, GLboolean transpose, const float* value);
			LE(glGetShaderiv); //TextureID shader, GLenum pname, GLint* param);
			LE(glLinkProgram); //TextureID program);
			LE(glGetProgramiv); //TextureID program, GLenum pname, GLint* param);
			LE(glBindVertexArray); //TextureID array);
			LE(glUniform2fv);
			LE(glUniform3f); //GLint location, float v0, float v1, float v2);
			LE(glUniform3fv); //GLint location, GLsizei count, const float* value);
			LE(glUniform4fv); //GLint location, GLsizei count, const float* value);
			LE(glBufferSubData); //GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid* data);
			LE(glGenVertexArrays); //GLsizei n, const TextureID* arrays);
			LE(glGetShaderInfoLog); //TextureID shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
			LE(glGetProgramInfoLog); //TextureID program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
			LE(glGetUniformBlockIndex); //TextureID program, const GLchar* uniformBlockName);
			LE(glUniformBlockBinding); //TextureID program, TextureID uniformBlockIndex, TextureID uniformBlockBinding);
			LE(glBindBufferBase); //GLenum target, TextureID index, TextureID buffer);
			LE(glTransformFeedbackVaryings); //TextureID, GLsizei, const GLchar **, GLenum);
			LE(glMapBuffer); //GLenum target, GLenum access);
			LE(glUnmapBuffer); //GLenum target);
			LE(glDrawElementsInstanced); //GLenum, GLsizei, GLenum, const GLvoid*, GLsizei);
			LE(glDrawArraysInstanced); //GLenum, GLint, GLsizei, GLsizei);
			LE(glDrawElementsInstancedBaseVertex); //GLenum mode, GLsizei count, GLenum type, const void* indices, GLsizei primcount, GLint basevertex);
			LE(glBeginTransformFeedback); //GLenum);
			LE(glEndTransformFeedback); //void);
			LE(glUniform1f); //GLint location, float v0);
			LE(glUniform2f); //GLint location, float v0, float v1);
			LE(glBlendEquationSeparate); //GLenum, GLenum);
			LE(glBlendFuncSeparate); //GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha);
			LE(glGetBufferSubData); //GLenum target, GLintptr offset, GLsizeiptr size, GLvoid* data);
			LE(glGetShaderSource);
			LE(glIsProgram);
			LE(glGetAttachedShaders);
			LE(glDrawBuffers);
			LE(glBlitFramebuffer);
			LE(glBlendEquation);
			LE(glBindSampler);
			LE(glDetachShader);

			return mExtensionsPresent;
		}

#ifdef IMGUI_API
		// Data
		static double       g_Time;
		static bool         g_MousePressed[3];
		static float        g_MouseWheel;
		static GLuint       g_FontTexture;
		static int          g_ShaderHandle, g_VertHandle, g_FragHandle;
		static int          g_AttribLocationTex, g_AttribLocationProjMtx;
		static int          g_AttribLocationPosition, g_AttribLocationUV, g_AttribLocationColor;
		static unsigned int g_VboHandle, g_VaoHandle, g_ElementsHandle;

		static bool IsAnyMouseButtonDown()
		{
			ImGuiIO& io = ImGui::GetIO();
			for (int n = 0; n < ARRAYSIZE(io.MouseDown); n++)
				if (io.MouseDown[n])
					return true;
			return false;
		}

		// We use the Win32 capture API (GetCapture/SetCapture/ReleaseCapture) to be able to read mouse coordinations when dragging mouse outside of our window bounds.
		static IMGUI_API LRESULT ImGui_WndProcHandler(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
		{
			ImGuiIO& io = ImGui::GetIO();
			switch (msg)
			{
			case WM_LBUTTONDOWN:
			case WM_RBUTTONDOWN:
			case WM_MBUTTONDOWN:
			{
				int button = 0;
				if (msg == WM_LBUTTONDOWN) button = 0;
				if (msg == WM_RBUTTONDOWN) button = 1;
				if (msg == WM_MBUTTONDOWN) button = 2;
				if (!IsAnyMouseButtonDown() && GetCapture() == NULL)
					SetCapture(hwnd);
				io.MouseDown[button] = true;
				return 0;
			}
			case WM_LBUTTONUP:
			case WM_RBUTTONUP:
			case WM_MBUTTONUP:
			{
				int button = 0;
				if (msg == WM_LBUTTONUP) button = 0;
				if (msg == WM_RBUTTONUP) button = 1;
				if (msg == WM_MBUTTONUP) button = 2;
				io.MouseDown[button] = false;
				if (!IsAnyMouseButtonDown() && GetCapture() == hwnd)
					ReleaseCapture();
				return 0;
			}
			case WM_MOUSEWHEEL:
				io.MouseWheel += GET_WHEEL_DELTA_WPARAM(wParam) > 0 ? +1.0f : -1.0f;
				return 0;
			case WM_MOUSEMOVE:
				io.MousePos.x = (signed short)(lParam);
				io.MousePos.y = (signed short)(lParam >> 16);
				return 0;
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (wParam < 256)
					io.KeysDown[wParam] = 1;
				return 0;
			case WM_KEYUP:
			case WM_SYSKEYUP:
				if (wParam < 256)
					io.KeysDown[wParam] = 0;
				return 0;
			case WM_CHAR:
				// You can also use ToAscii()+GetKeyboardState() to retrieve characters.
				if (wParam > 0 && wParam < 0x10000)
					io.AddInputCharacter((unsigned short)wParam);
				return 0;
			}
			return 0;
		}
		// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
		// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so. 
		// If text or lines are blurry when integrating ImGui in your engine: in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
		static void ImGui_RenderDrawLists(ImDrawData* draw_data)
		{
			// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
			ImGuiIO& io = ImGui::GetIO();
			int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
			int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
			if (fb_width == 0 || fb_height == 0)
				return;
			draw_data->ScaleClipRects(io.DisplayFramebufferScale);

			// Backup GL state
			GLenum last_active_texture; glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint*)&last_active_texture);
			glActiveTexture(GL_TEXTURE0);
			GLint last_program; glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
			GLint last_texture; glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			GLint last_sampler; glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
			GLint last_array_buffer; glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
			GLint last_element_array_buffer; glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
			GLint last_vertex_array; glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
			GLint last_polygon_mode[2]; glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
			GLint last_viewport[4]; glGetIntegerv(GL_VIEWPORT, last_viewport);
			GLint last_scissor_box[4]; glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
			GLenum last_blend_src_rgb; glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
			GLenum last_blend_dst_rgb; glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
			GLenum last_blend_src_alpha; glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
			GLenum last_blend_dst_alpha; glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
			GLenum last_blend_equation_rgb; glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
			GLenum last_blend_equation_alpha; glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
			GLboolean last_enable_blend = glIsEnabled(GL_BLEND);
			GLboolean last_enable_cull_face = glIsEnabled(GL_CULL_FACE);
			GLboolean last_enable_depth_test = glIsEnabled(GL_DEPTH_TEST);
			GLboolean last_enable_scissor_test = glIsEnabled(GL_SCISSOR_TEST);

			// Setup render state: alpha-blending enabled, no face culling, no depth testing, scissor enabled, polygon fill
			glEnable(GL_BLEND);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			glDisable(GL_CULL_FACE);
			glDisable(GL_DEPTH_TEST);
			glEnable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

			// Setup viewport, orthographic projection matrix
			glViewport(0, 0, (GLsizei)fb_width, (GLsizei)fb_height);
			const float ortho_projection[4][4] =
			{
				{ 2.0f / io.DisplaySize.x, 0.0f,                   0.0f, 0.0f },
				{ 0.0f,                  2.0f / -io.DisplaySize.y, 0.0f, 0.0f },
				{ 0.0f,                  0.0f,                  -1.0f, 0.0f },
				{ -1.0f,                  1.0f,                   0.0f, 1.0f },
			};
			glUseProgram(g_ShaderHandle);
			glUniform1i(g_AttribLocationTex, 0);
			glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
			glBindVertexArray(g_VaoHandle);
			glBindSampler(0, 0); // Rely on combined texture/sampler state.

			for (int n = 0; n < draw_data->CmdListsCount; n++)
			{
				const ImDrawList* cmd_list = draw_data->CmdLists[n];
				const ImDrawIdx* idx_buffer_offset = 0;

				glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
				glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

				for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
				{
					const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
					if (pcmd->UserCallback)
					{
						pcmd->UserCallback(cmd_list, pcmd);
					}
					else
					{
						glBindTexture(GL_TEXTURE_2D, (GLuint)(intptr_t)pcmd->TextureId);
						glScissor((int)pcmd->ClipRect.x, (int)(fb_height - pcmd->ClipRect.w), (int)(pcmd->ClipRect.z - pcmd->ClipRect.x), (int)(pcmd->ClipRect.w - pcmd->ClipRect.y));
						glDrawElements(GL_TRIANGLES, (GLsizei)pcmd->ElemCount, sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT, idx_buffer_offset);
					}
					idx_buffer_offset += pcmd->ElemCount;
				}
			}

			// Restore modified GL state
			glUseProgram(last_program);
			glBindTexture(GL_TEXTURE_2D, last_texture);
			glBindSampler(0, last_sampler);
			glActiveTexture(last_active_texture);
			glBindVertexArray(last_vertex_array);
			glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, last_element_array_buffer);
			glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
			glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb, last_blend_src_alpha, last_blend_dst_alpha);
			if (last_enable_blend) glEnable(GL_BLEND); else glDisable(GL_BLEND);
			if (last_enable_cull_face) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
			if (last_enable_depth_test) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
			if (last_enable_scissor_test) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
			glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
			glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
			glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
		}
#if 0
		static const char* ImGui_GetClipboardText(void* user_data)
		{
			return glfwGetClipboardString((GLFWwindow*)user_data);
		}

		static void ImGui_SetClipboardText(void* user_data, const char* text)
		{
			glfwSetClipboardString((GLFWwindow*)user_data, text);
		}

		void ImGui_MouseButtonCallback(GLFWwindow*, int button, int action, int /*mods*/)
		{
			if (action == GLFW_PRESS && button >= 0 && button < 3)
				g_MousePressed[button] = true;
		}

		void ImGui_ScrollCallback(GLFWwindow*, double /*xoffset*/, double yoffset)
		{
			g_MouseWheel += (float)yoffset; // Use fractional mouse wheel, 1.0 unit 5 lines.
		}

		void ImGui_KeyCallback(GLFWwindow*, int key, int, int action, int mods)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (action == GLFW_PRESS)
				io.KeysDown[key] = true;
			if (action == GLFW_RELEASE)
				io.KeysDown[key] = false;

			(void)mods; // Modifiers are not reliable across systems
			io.KeyCtrl = io.KeysDown[GLFW_KEY_LEFT_CONTROL] || io.KeysDown[GLFW_KEY_RIGHT_CONTROL];
			io.KeyShift = io.KeysDown[GLFW_KEY_LEFT_SHIFT] || io.KeysDown[GLFW_KEY_RIGHT_SHIFT];
			io.KeyAlt = io.KeysDown[GLFW_KEY_LEFT_ALT] || io.KeysDown[GLFW_KEY_RIGHT_ALT];
			io.KeySuper = io.KeysDown[GLFW_KEY_LEFT_SUPER] || io.KeysDown[GLFW_KEY_RIGHT_SUPER];
		}

		void ImGui_CharCallback(GLFWwindow*, unsigned int c)
		{
			ImGuiIO& io = ImGui::GetIO();
			if (c > 0 && c < 0x10000)
				io.AddInputCharacter((unsigned short)c);
		}
#endif
		bool ImGui_CreateFontsTexture()
		{
			// Build texture atlas
			ImGuiIO& io = ImGui::GetIO();
			unsigned char* pixels;
			int width, height;
			io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);   // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

																	  // Upload texture to graphics system
			GLint last_texture;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGenTextures(1, &g_FontTexture);
			glBindTexture(GL_TEXTURE_2D, g_FontTexture);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

			// Store our identifier
			io.Fonts->TexID = (void *)(intptr_t)g_FontTexture;

			// Restore state
			glBindTexture(GL_TEXTURE_2D, last_texture);

			return true;
		}

		bool ImGui_CreateDeviceObjects()
		{
			// Backup GL state
			GLint last_texture, last_array_buffer, last_vertex_array;
			glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
			glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
			glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);

			const GLchar *vertex_shader =
				"#version 330\n"
				"uniform mat4 ProjMtx;\n"
				"in vec2 Position;\n"
				"in vec2 UV;\n"
				"in vec4 Color;\n"
				"out vec2 Frag_UV;\n"
				"out vec4 Frag_Color;\n"
				"void main()\n"
				"{\n"
				"	Frag_UV = UV;\n"
				"	Frag_Color = Color;\n"
				"	gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
				"}\n";

			const GLchar* fragment_shader =
				"#version 330\n"
				"uniform sampler2D Texture;\n"
				"in vec2 Frag_UV;\n"
				"in vec4 Frag_Color;\n"
				"out vec4 Out_Color;\n"
				"void main()\n"
				"{\n"
				"	Out_Color = Frag_Color * texture( Texture, Frag_UV.st);\n"
				"}\n";

			g_ShaderHandle = glCreateProgram();
			g_VertHandle = glCreateShader(GL_VERTEX_SHADER);
			g_FragHandle = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(g_VertHandle, 1, &vertex_shader, 0);
			glShaderSource(g_FragHandle, 1, &fragment_shader, 0);
			glCompileShader(g_VertHandle);
			glCompileShader(g_FragHandle);
			glAttachShader(g_ShaderHandle, g_VertHandle);
			glAttachShader(g_ShaderHandle, g_FragHandle);
			glLinkProgram(g_ShaderHandle);

			g_AttribLocationTex = glGetUniformLocation(g_ShaderHandle, "Texture");
			g_AttribLocationProjMtx = glGetUniformLocation(g_ShaderHandle, "ProjMtx");
			g_AttribLocationPosition = glGetAttribLocation(g_ShaderHandle, "Position");
			g_AttribLocationUV = glGetAttribLocation(g_ShaderHandle, "UV");
			g_AttribLocationColor = glGetAttribLocation(g_ShaderHandle, "Color");

			glGenBuffers(1, &g_VboHandle);
			glGenBuffers(1, &g_ElementsHandle);

			glGenVertexArrays(1, &g_VaoHandle);
			glBindVertexArray(g_VaoHandle);
			glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
			glEnableVertexAttribArray(g_AttribLocationPosition);
			glEnableVertexAttribArray(g_AttribLocationUV);
			glEnableVertexAttribArray(g_AttribLocationColor);
#ifndef OFFSETOF
#define OFFSETOF(TYPE, ELEMENT) ((size_t)&(((TYPE *)0)->ELEMENT))
#endif
			glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, pos));
			glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, uv));
			glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid*)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

			ImGui_CreateFontsTexture();

			// Restore modified GL state
			glBindTexture(GL_TEXTURE_2D, last_texture);
			glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
			glBindVertexArray(last_vertex_array);

			return true;
		}

		void    ImGui_InvalidateDeviceObjects()
		{
			if (g_VaoHandle) glDeleteVertexArrays(1, &g_VaoHandle);
			if (g_VboHandle) glDeleteBuffers(1, &g_VboHandle);
			if (g_ElementsHandle) glDeleteBuffers(1, &g_ElementsHandle);
			g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

			if (g_ShaderHandle && g_VertHandle) glDetachShader(g_ShaderHandle, g_VertHandle);
			if (g_VertHandle) glDeleteShader(g_VertHandle);
			g_VertHandle = 0;

			if (g_ShaderHandle && g_FragHandle) glDetachShader(g_ShaderHandle, g_FragHandle);
			if (g_FragHandle) glDeleteShader(g_FragHandle);
			g_FragHandle = 0;

			if (g_ShaderHandle) glDeleteProgram(g_ShaderHandle);
			g_ShaderHandle = 0;

			if (g_FontTexture)
			{
				glDeleteTextures(1, &g_FontTexture);
				ImGui::GetIO().Fonts->TexID = 0;
				g_FontTexture = 0;
			}
		}

		bool    ImGui_Init()
		{
			ImGuiIO& io = ImGui::GetIO();
			io.KeyMap[ImGuiKey_Tab] = VK_TAB;                       // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
			io.KeyMap[ImGuiKey_LeftArrow] = VK_LEFT;
			io.KeyMap[ImGuiKey_RightArrow] = VK_RIGHT;
			io.KeyMap[ImGuiKey_UpArrow] = VK_UP;
			io.KeyMap[ImGuiKey_DownArrow] = VK_DOWN;
			io.KeyMap[ImGuiKey_PageUp] = VK_PRIOR;
			io.KeyMap[ImGuiKey_PageDown] = VK_NEXT;
			io.KeyMap[ImGuiKey_Home] = VK_HOME;
			io.KeyMap[ImGuiKey_End] = VK_END;
			io.KeyMap[ImGuiKey_Delete] = VK_DELETE;
			io.KeyMap[ImGuiKey_Backspace] = VK_BACK;
			io.KeyMap[ImGuiKey_Enter] = VK_RETURN;
			io.KeyMap[ImGuiKey_Escape] = VK_ESCAPE;
			io.KeyMap[ImGuiKey_A] = 'A';
			io.KeyMap[ImGuiKey_C] = 'C';
			io.KeyMap[ImGuiKey_V] = 'V';
			io.KeyMap[ImGuiKey_X] = 'X';
			io.KeyMap[ImGuiKey_Y] = 'Y';
			io.KeyMap[ImGuiKey_Z] = 'Z';

			io.ImeWindowHandle = this->wininfo.hWnd;
			io.RenderDrawListsFn = ImGui_RenderDrawLists;       // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
			/*
			io.SetClipboardTextFn = ImGui_SetClipboardText;
			io.GetClipboardTextFn = ImGui_GetClipboardText;
			//io.ClipboardUserData = g_Window;
			*/
			return true;
		}

		void ImGui_Shutdown()
		{
			ImGui_InvalidateDeviceObjects();
		}

		void ImGui_NewFrame()
		{
			if (!g_FontTexture)
				ImGui_CreateDeviceObjects();

			ImGuiIO& io = ImGui::GetIO();

			// Read keyboard modifiers inputs
			io.KeyCtrl = (GetKeyState(VK_CONTROL) & 0x8000) != 0;
			io.KeyShift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
			io.KeyAlt = (GetKeyState(VK_MENU) & 0x8000) != 0;
			io.KeySuper = false;

			/*
			// Setup display size (every frame to accommodate for window resizing)
			int w, h;
			int display_w, display_h;
			glfwGetWindowSize(g_Window, &w, &h);
			glfwGetFramebufferSize(g_Window, &display_w, &display_h);
			io.DisplaySize = ImVec2((float)w, (float)h);
			io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);

			// Setup time step
			double current_time = glfwGetTime();
			io.DeltaTime = g_Time > 0.0 ? (float)(current_time - g_Time) : (float)(1.0f / 60.0f);
			g_Time = current_time;

			// Setup inputs
			// (we already got mouse wheel, keyboard keys & characters from glfw callbacks polled in glfwPollEvents())
			if (glfwGetWindowAttrib(g_Window, GLFW_FOCUSED))
			{
				if (io.WantMoveMouse)
				{
					glfwSetCursorPos(g_Window, (double)io.MousePos.x, (double)io.MousePos.y);   // Set mouse position if requested by io.WantMoveMouse flag (used when io.NavMovesTrue is enabled by user and using directional navigation)
				}
				else
				{
					double mouse_x, mouse_y;
					glfwGetCursorPos(g_Window, &mouse_x, &mouse_y);
					io.MousePos = ImVec2((float)mouse_x, (float)mouse_y);   // Get mouse position in screen coordinates (set to -1,-1 if no mouse / on another screen, etc.)
				}
			}
			else
			{
				io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
			}

			for (int i = 0; i < 3; i++)
			{
				io.MouseDown[i] = g_MousePressed[i] || glfwGetMouseButton(g_Window, i) != 0;    // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
				g_MousePressed[i] = false;
			}

			io.MouseWheel = g_MouseWheel;
			g_MouseWheel = 0.0f;

			// Hide OS mouse cursor if ImGui is drawing it
			glfwSetInputMode(g_Window, GLFW_CURSOR, io.MouseDrawCursor ? GLFW_CURSOR_HIDDEN : GLFW_CURSOR_NORMAL);
			*/
			// Start the frame
			ImGui::NewFrame();
		}




#endif // IMGUI_API

	}; // IMAPP

#ifdef IMAPP_IMPL
	ImApp *ImApp::mInstance = NULL;
#ifdef IMGUI_API

	double       ImApp::g_Time = 0.0f;
	bool         ImApp::g_MousePressed[3] = { false, false, false };
	float        ImApp::g_MouseWheel = 0.0f;
	GLuint       ImApp::g_FontTexture = 0;
	int          ImApp::g_ShaderHandle = 0, ImApp::g_VertHandle = 0, ImApp::g_FragHandle = 0;
	int          ImApp::g_AttribLocationTex = 0, ImApp::g_AttribLocationProjMtx = 0;
	int          ImApp::g_AttribLocationPosition = 0, ImApp::g_AttribLocationUV = 0, ImApp::g_AttribLocationColor = 0;
	unsigned int ImApp::g_VboHandle = 0, ImApp::g_VaoHandle = 0, ImApp::g_ElementsHandle = 0;
#endif

	void ImApp::LoadBanks(int bankCount, const char **bankPaths) {}
	void ImApp::PlayEvent(const char *eventName) {}

#endif
}