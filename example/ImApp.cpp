#include "ImApp.h"
#include "glutil.h"
#include "Win32Window.h"
#include "ImGuiImplScreenState.h"
#include "Renderer.h"
#include <imgui.h>

// Data
static double g_Time = {};
static bool g_MousePressed[3] = {};
static float g_MouseWheel = {};

static void ImGui_Impl_Win32_UpdateMouseCursor()
{
	ImGuiIO &io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange)
	{
		SetCursor(LoadCursor(NULL, IDC_ARROW));
		return;
	}

	ImGuiMouseCursor imgui_cursor = ImGui::GetMouseCursor();
	if (imgui_cursor == ImGuiMouseCursor_None || io.MouseDrawCursor)
	{
		// Hide OS mouse cursor if imgui is drawing it or if it wants no cursor
		::SetCursor(NULL);
		return;
	}

	// Show OS mouse cursor
	LPTSTR win32_cursor = IDC_ARROW;
	switch (imgui_cursor)
	{
	case ImGuiMouseCursor_Arrow:
		win32_cursor = IDC_ARROW;
		break;
	case ImGuiMouseCursor_TextInput:
		win32_cursor = IDC_IBEAM;
		break;
	case ImGuiMouseCursor_ResizeAll:
		win32_cursor = IDC_SIZEALL;
		break;
	case ImGuiMouseCursor_ResizeEW:
		win32_cursor = IDC_SIZEWE;
		break;
	case ImGuiMouseCursor_ResizeNS:
		win32_cursor = IDC_SIZENS;
		break;
	case ImGuiMouseCursor_ResizeNESW:
		win32_cursor = IDC_SIZENESW;
		break;
	case ImGuiMouseCursor_ResizeNWSE:
		win32_cursor = IDC_SIZENWSE;
		break;
	case ImGuiMouseCursor_Hand:
		win32_cursor = IDC_HAND;
		break;
		// case ImGuiMouseCursor_NotAllowed:
		//     win32_cursor = IDC_NO;
		//     break;
	}
	::SetCursor(::LoadCursor(NULL, win32_cursor));
}

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
		// MSG msg;
		// while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		// {
		// 	TranslateMessage(&msg);
		// 	DispatchMessage(&msg);

		// 	if (msg.message == WM_QUIT)
		// 		mDone = true;
		// }
		screenstate::ScreenState state;
		if (!m_window.Update(&state))
		{
			mDone = true;
		}

		// Start the frame
		ImGui_Impl_ScreenState_NewFrame(state);
		ImGui_Impl_Win32_UpdateMouseCursor();
		ImGui::NewFrame();
	}

	void EndFrame()
	{
		ImGui::Render();

		SwapBuffers(wininfo.hDC);
	}

	void Finish()
	{
		ImGui_InvalidateDeviceObjects();
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

	bool ImGui_Init()
	{
		ImGuiIO &io = ImGui::GetIO();
		ImGui_Impl_ScreenState_Init();
		// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
		io.RenderDrawListsFn = ImGui_RenderDrawLists;
		return true;
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
