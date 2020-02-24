#include "ImApp.h"
#include "glutil.h"
#include "Win32Window.h"
#include <imgui.h>
#define IMGUI_API
#define IMAPP_IMPL

// Data
static double g_Time = {};
static bool g_MousePressed[3] = {};
static float g_MouseWheel = {};
static GLuint g_FontTexture = {};
static int g_ShaderHandle = {}, g_VertHandle = {}, g_FragHandle = {};
static int g_AttribLocationTex = {}, g_AttribLocationProjMtx = {};
static int g_AttribLocationPosition = {}, g_AttribLocationUV = {}, g_AttribLocationColor = {};
static unsigned int g_VboHandle = {}, g_VaoHandle = {}, g_ElementsHandle = {};

class Impl
{
	ImApp::Config mConfig = {};
	bool mDone = true;
	typedef struct
	{
		//---------------
		HINSTANCE hInstance;
		HDC hDC;
		HGLRC hRC;
		HWND hWnd;
		char wndclass[4]; // window class and title :)
						  //---------------
	} WININFO;
	WININFO wininfo = {};

	screenstate::Win32Window m_window;

public:
	Impl()
		: m_window(L"ImGuizmoExampleWindow")
	{
	}

	int Init(const ImApp::Config &config)
	{
		mConfig = config;
		wininfo = WININFO{0, 0, 0, 0, {'c', 'X', 'd', 0}};
		WININFO *info = &wininfo;

		info->hInstance = GetModuleHandle(0);

		ImGui::CreateContext();
		if (!WindowInit(info))
		{
			return 0;
		}

		if (!::InitExtension())
		{
			return 0;
		}

		if (!ImGui_Init())
		{
			return 0;
		}

		mDone = false;
		return 1;
	}

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
	int WindowInit(WININFO *info)
	{
		info->hWnd = m_window.Create(L"ImGuizmoExample");

		if (!info->hWnd)
			return (0);

		if (!(info->hDC = GetDC(info->hWnd)))
			return (0);

		static PIXELFORMATDESCRIPTOR pfd =
			{
				sizeof(PIXELFORMATDESCRIPTOR),
				1,
				PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
				PFD_TYPE_RGBA,
				32,
				0, 0, 0, 0, 0, 0, 8, 0,
				0, 0, 0, 0, 0, // accum
				32,			   // zbuffer
				0,			   // stencil!
				0,			   // aux
				PFD_MAIN_PLANE,
				0, 0, 0, 0};

		unsigned int PixelFormat;
		if (!(PixelFormat = ChoosePixelFormat(info->hDC, &pfd)))
			return (0);

		if (!SetPixelFormat(info->hDC, PixelFormat, &pfd))
			return (0);

		if (!(info->hRC = wglCreateContext(info->hDC)))
			return (0);

		if (!wglMakeCurrent(info->hDC, info->hRC))
			return (0);

		return (1);
	}

	// This is the main rendering function that you have to implement and provide to ImGui (via setting up 'RenderDrawListsFn' in the ImGuiIO structure)
	// Note that this implementation is little overcomplicated because we are saving/setting up/restoring every OpenGL state explicitly, in order to be able to run within any OpenGL engine that doesn't do so.
	// If text or lines are blurry when integrating ImGui in your engine: in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
	static void ImGui_RenderDrawLists(ImDrawData *draw_data)
	{
		// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer coordinates)
		ImGuiIO &io = ImGui::GetIO();
		int fb_width = (int)(io.DisplaySize.x * io.DisplayFramebufferScale.x);
		int fb_height = (int)(io.DisplaySize.y * io.DisplayFramebufferScale.y);
		if (fb_width == 0 || fb_height == 0)
			return;
		draw_data->ScaleClipRects(io.DisplayFramebufferScale);

		// Backup GL state
		GLenum last_active_texture;
		glGetIntegerv(GL_ACTIVE_TEXTURE, (GLint *)&last_active_texture);
		glActiveTexture(GL_TEXTURE0);
		GLint last_program;
		glGetIntegerv(GL_CURRENT_PROGRAM, &last_program);
		GLint last_texture;
		glGetIntegerv(GL_TEXTURE_BINDING_2D, &last_texture);
		GLint last_sampler;
		glGetIntegerv(GL_SAMPLER_BINDING, &last_sampler);
		GLint last_array_buffer;
		glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &last_array_buffer);
		GLint last_element_array_buffer;
		glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &last_element_array_buffer);
		GLint last_vertex_array;
		glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &last_vertex_array);
		GLint last_polygon_mode[2];
		glGetIntegerv(GL_POLYGON_MODE, last_polygon_mode);
		GLint last_viewport[4];
		glGetIntegerv(GL_VIEWPORT, last_viewport);
		GLint last_scissor_box[4];
		glGetIntegerv(GL_SCISSOR_BOX, last_scissor_box);
		GLenum last_blend_src_rgb;
		glGetIntegerv(GL_BLEND_SRC_RGB, (GLint *)&last_blend_src_rgb);
		GLenum last_blend_dst_rgb;
		glGetIntegerv(GL_BLEND_DST_RGB, (GLint *)&last_blend_dst_rgb);
		GLenum last_blend_src_alpha;
		glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint *)&last_blend_src_alpha);
		GLenum last_blend_dst_alpha;
		glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint *)&last_blend_dst_alpha);
		GLenum last_blend_equation_rgb;
		glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint *)&last_blend_equation_rgb);
		GLenum last_blend_equation_alpha;
		glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint *)&last_blend_equation_alpha);
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
				{2.0f / io.DisplaySize.x, 0.0f, 0.0f, 0.0f},
				{0.0f, 2.0f / -io.DisplaySize.y, 0.0f, 0.0f},
				{0.0f, 0.0f, -1.0f, 0.0f},
				{-1.0f, 1.0f, 0.0f, 1.0f},
			};
		glUseProgram(g_ShaderHandle);
		glUniform1i(g_AttribLocationTex, 0);
		glUniformMatrix4fv(g_AttribLocationProjMtx, 1, GL_FALSE, &ortho_projection[0][0]);
		glBindVertexArray(g_VaoHandle);
		glBindSampler(0, 0); // Rely on combined texture/sampler state.

		for (int n = 0; n < draw_data->CmdListsCount; n++)
		{
			const ImDrawList *cmd_list = draw_data->CmdLists[n];
			const ImDrawIdx *idx_buffer_offset = 0;

			glBindBuffer(GL_ARRAY_BUFFER, g_VboHandle);
			glBufferData(GL_ARRAY_BUFFER, (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert), (const GLvoid *)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_ElementsHandle);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx), (const GLvoid *)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

			for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++)
			{
				const ImDrawCmd *pcmd = &cmd_list->CmdBuffer[cmd_i];
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
		if (last_enable_blend)
			glEnable(GL_BLEND);
		else
			glDisable(GL_BLEND);
		if (last_enable_cull_face)
			glEnable(GL_CULL_FACE);
		else
			glDisable(GL_CULL_FACE);
		if (last_enable_depth_test)
			glEnable(GL_DEPTH_TEST);
		else
			glDisable(GL_DEPTH_TEST);
		if (last_enable_scissor_test)
			glEnable(GL_SCISSOR_TEST);
		else
			glDisable(GL_SCISSOR_TEST);
		glPolygonMode(GL_FRONT_AND_BACK, last_polygon_mode[0]);
		glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);
		glScissor(last_scissor_box[0], last_scissor_box[1], (GLsizei)last_scissor_box[2], (GLsizei)last_scissor_box[3]);
	}

	bool ImGui_CreateFontsTexture()
	{
		// Build texture atlas
		ImGuiIO &io = ImGui::GetIO();
		unsigned char *pixels;
		int width, height;
		io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height); // Load as RGBA 32-bits (75% of the memory is wasted, but default font is so small) because it is more likely to be compatible with user's existing shaders. If your ImTextureId represent a higher-level concept than just a GL texture id, consider calling GetTexDataAsAlpha8() instead to save on GPU memory.

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

		const GLchar *fragment_shader =
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
#define OFFSETOF(TYPE, ELEMENT) ((size_t) & (((TYPE *)0)->ELEMENT))
#endif
		glVertexAttribPointer(g_AttribLocationPosition, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, pos));
		glVertexAttribPointer(g_AttribLocationUV, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, uv));
		glVertexAttribPointer(g_AttribLocationColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert), (GLvoid *)OFFSETOF(ImDrawVert, col));
#undef OFFSETOF

		ImGui_CreateFontsTexture();

		// Restore modified GL state
		glBindTexture(GL_TEXTURE_2D, last_texture);
		glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
		glBindVertexArray(last_vertex_array);

		return true;
	}

	void ImGui_InvalidateDeviceObjects()
	{
		if (g_VaoHandle)
			glDeleteVertexArrays(1, &g_VaoHandle);
		if (g_VboHandle)
			glDeleteBuffers(1, &g_VboHandle);
		if (g_ElementsHandle)
			glDeleteBuffers(1, &g_ElementsHandle);
		g_VaoHandle = g_VboHandle = g_ElementsHandle = 0;

		if (g_ShaderHandle && g_VertHandle)
			glDetachShader(g_ShaderHandle, g_VertHandle);
		if (g_VertHandle)
			glDeleteShader(g_VertHandle);
		g_VertHandle = 0;

		if (g_ShaderHandle && g_FragHandle)
			glDetachShader(g_ShaderHandle, g_FragHandle);
		if (g_FragHandle)
			glDeleteShader(g_FragHandle);
		g_FragHandle = 0;

		if (g_ShaderHandle)
			glDeleteProgram(g_ShaderHandle);
		g_ShaderHandle = 0;

		if (g_FontTexture)
		{
			glDeleteTextures(1, &g_FontTexture);
			ImGui::GetIO().Fonts->TexID = 0;
			g_FontTexture = 0;
		}
	}

	bool ImGui_Init()
	{
		ImGuiIO &io = ImGui::GetIO();
		io.KeyMap[ImGuiKey_Tab] = VK_TAB; // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array that we will update during the application lifetime.
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
		io.RenderDrawListsFn = ImGui_RenderDrawLists; // Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
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

		ImGuiIO &io = ImGui::GetIO();

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
};

namespace ImApp
{

ImApp::ImApp()
	: m_impl(new Impl)
{
}

ImApp::~ImApp()
{
	delete m_impl;
}

int ImApp::Init(const Config &config)
{
	return m_impl->Init(config);
}

void ImApp::NewFrame()
{
	m_impl->NewFrame();
}

void ImApp::EndFrame()
{
	m_impl->EndFrame();
}

void ImApp::Finish()
{
	m_impl->Finish();
}

bool ImApp::Done()
{
	return m_impl->Done();
}

} // namespace ImApp
