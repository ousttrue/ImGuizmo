#include "ImApp.h"
#include "glutil.h"
#include "Win32Window.h"
#include "ImGuiImplScreenState.h"
#include "Renderer.h"
#include "wgl.h"
#include <imgui.h>


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
	screenstate::Win32Window m_window;
	WGL m_wgl;

public:
	Impl()
		: m_window(L"ImGuizmoExampleWindow")
	{
	}

	bool Initialize()
	{
		ImGui::CreateContext();

		auto hwnd = m_window.Create(L"ImGuizmoExample");
		if (!hwnd)
		{
			return false;
		}

		if (!m_wgl.Initialize(hwnd))
		{
			return false;
		}

		return (1);

		if (!::InitExtension())
		{
			return false;
		}

		ImGuiIO &io = ImGui::GetIO();
		ImGui_Impl_ScreenState_Init();
		// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
		io.RenderDrawListsFn = ImGui_RenderDrawLists;
		return true;
	}

	bool NewFrame()
	{
		screenstate::ScreenState state;
		if (!m_window.Update(&state))
		{
			return false;
		}

		// Start the frame
		ImGui_Impl_ScreenState_NewFrame(state);
		ImGui_Impl_Win32_UpdateMouseCursor();
		ImGui::NewFrame();

		return true;
	}

	void EndFrame()
	{
		ImGui::Render();
		m_wgl.Present();
	}

	void Finish()
	{
		ImGui_InvalidateDeviceObjects();
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

int ImApp::Init()
{
	return m_impl->Initialize();
}

bool ImApp::NewFrame()
{
	return m_impl->NewFrame();
}

void ImApp::EndFrame()
{
	m_impl->EndFrame();
}

void ImApp::Finish()
{
	m_impl->Finish();
}


} // namespace ImApp
