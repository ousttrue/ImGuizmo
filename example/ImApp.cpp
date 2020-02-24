#include "ImApp.h"
#include "glutil.h"
#include "Win32Window.h"
#include "ImGuiImplScreenState.h"
#include "Renderer.h"
#include "wgl.h"
#include <imgui.h>




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
