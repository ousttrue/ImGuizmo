#include "camera.h"
#include "MySequence.h"
#include "imgui.h"
#define IMAPP_IMPL
#include "glutil.h"
#include "Win32Window.h"
#include "ImGuiImplScreenState.h"
#include "Renderer.h"
#include "wgl.h"
#include "ImGuizmo.h"
#include <math.h>
#include <vector>
#include <algorithm>
#include "imgui_internal.h"
//
//
// ImGuizmo example helper functions
//
//

static const float identityMatrix[16] =
	{1.f, 0.f, 0.f, 0.f,
	 0.f, 1.f, 0.f, 0.f,
	 0.f, 0.f, 1.f, 0.f,
	 0.f, 0.f, 0.f, 1.f};


void EditTransform(const float *cameraView, float *cameraProjection, float *matrix)
{
	static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
	static bool useSnap = false;
	static float snap[3] = {1.f, 1.f, 1.f};
	static float bounds[] = {-0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f};
	static float boundsSnap[] = {0.1f, 0.1f, 0.1f};
	static bool boundSizing = false;
	static bool boundSizingSnap = false;

	if (ImGui::IsKeyPressed(90))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	if (ImGui::IsKeyPressed(69))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	if (ImGui::IsKeyPressed(82)) // r Key
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	if (ImGui::RadioButton("Translate", mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
		mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Rotate", mCurrentGizmoOperation == ImGuizmo::ROTATE))
		mCurrentGizmoOperation = ImGuizmo::ROTATE;
	ImGui::SameLine();
	if (ImGui::RadioButton("Scale", mCurrentGizmoOperation == ImGuizmo::SCALE))
		mCurrentGizmoOperation = ImGuizmo::SCALE;
	float matrixTranslation[3], matrixRotation[3], matrixScale[3];
	ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
	ImGui::InputFloat3("Tr", matrixTranslation, 3);
	ImGui::InputFloat3("Rt", matrixRotation, 3);
	ImGui::InputFloat3("Sc", matrixScale, 3);
	ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

	if (mCurrentGizmoOperation != ImGuizmo::SCALE)
	{
		if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
			mCurrentGizmoMode = ImGuizmo::LOCAL;
		ImGui::SameLine();
		if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
			mCurrentGizmoMode = ImGuizmo::WORLD;
	}
	if (ImGui::IsKeyPressed(83))
		useSnap = !useSnap;
	ImGui::Checkbox("", &useSnap);
	ImGui::SameLine();

	switch (mCurrentGizmoOperation)
	{
	case ImGuizmo::TRANSLATE:
		ImGui::InputFloat3("Snap", &snap[0]);
		break;
	case ImGuizmo::ROTATE:
		ImGui::InputFloat("Angle Snap", &snap[0]);
		break;
	case ImGuizmo::SCALE:
		ImGui::InputFloat("Scale Snap", &snap[0]);
		break;
	}
	ImGui::Checkbox("Bound Sizing", &boundSizing);
	if (boundSizing)
	{
		ImGui::PushID(3);
		ImGui::Checkbox("", &boundSizingSnap);
		ImGui::SameLine();
		ImGui::InputFloat3("Snap", boundsSnap);
		ImGui::PopID();
	}

	ImGuiIO &io = ImGui::GetIO();
	ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
	ImGuizmo::Manipulate(cameraView, cameraProjection, mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
}

inline void rotationY(const float angle, float *m16)
{
	float c = cosf(angle);
	float s = sinf(angle);

	m16[0] = c;
	m16[1] = 0.0f;
	m16[2] = -s;
	m16[3] = 0.0f;
	m16[4] = 0.0f;
	m16[5] = 1.f;
	m16[6] = 0.0f;
	m16[7] = 0.0f;
	m16[8] = s;
	m16[9] = 0.0f;
	m16[10] = c;
	m16[11] = 0.0f;
	m16[12] = 0.f;
	m16[13] = 0.f;
	m16[14] = 0.f;
	m16[15] = 1.0f;
}

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


int main(int, char **)
{
	float objectMatrix[16] =
		{1.f, 0.f, 0.f, 0.f,
		 0.f, 1.f, 0.f, 0.f,
		 0.f, 0.f, 1.f, 0.f,
		 0.f, 0.f, 0.f, 1.f};

	// sequence with default values
	MySequence mySequence;
	mySequence.mFrameMin = -100;
	mySequence.mFrameMax = 1000;
	mySequence.myItems.push_back(MySequence::MySequenceItem{0, 10, 30, false});
	mySequence.myItems.push_back(MySequence::MySequenceItem{1, 20, 30, true});
	mySequence.myItems.push_back(MySequence::MySequenceItem{3, 12, 60, false});
	mySequence.myItems.push_back(MySequence::MySequenceItem{2, 61, 90, false});
	mySequence.myItems.push_back(MySequence::MySequenceItem{4, 90, 99, false});

	// Camera projection
	rotationY(0.f, objectMatrix);

	screenstate::Win32Window m_window(L"ImGuizmoExampleWindow");
	auto hwnd = m_window.Create(L"ImGuizmoExample");
	if (!hwnd)
	{
		return false;
	}

	WGL m_wgl;
	if (!m_wgl.Initialize(hwnd))
	{
		return false;
	}
	if (!::InitExtension())
	{
		return false;
	}

	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();
	ImGui_Impl_ScreenState_Init();
	// Alternatively you can set this to NULL and call ImGui::GetDrawData() after ImGui::Render() to get the same ImDrawData pointer.
	io.RenderDrawListsFn = ImGui_RenderDrawLists;

	// create texture
	ImGui_CreateDeviceObjects();

	m_window.Show();

	Camera camera;

	// Main loop
	screenstate::ScreenState state;
	while (m_window.Update(&state))
	{
		// Start the frame
		ImGui_Impl_ScreenState_NewFrame(state);
		ImGui::NewFrame();
		ImGui_Impl_Win32_UpdateMouseCursor();

		ImGuiIO &io = ImGui::GetIO();
		camera.Update(io.DisplaySize.x, io.DisplaySize.y);
		ImGuizmo::SetOrthographic(!camera.isPerspective);

		ImGuizmo::BeginFrame();

		ImGui::Begin("Editor");
		ImGui::Text("Camera");
		camera.Gui();
		ImGuizmo::DrawCube(camera.cameraView, camera.cameraProjection, objectMatrix);
		ImGuizmo::DrawGrid(camera.cameraView, camera.cameraProjection, identityMatrix, 10.f);
		ImGui::Text("X: %f Y: %f", io.MousePos.x, io.MousePos.y);
		ImGui::Separator();
		EditTransform(camera.cameraView, camera.cameraProjection, objectMatrix);
		ImGui::End();

		// let's create the sequencer
		mySequence.Gui();
		ImGuizmo::ViewManipulate(camera.cameraView, camera.camDistance, ImVec2(io.DisplaySize.x - 128, 0), ImVec2(128, 128), 0x10101010);

		// render everything
		glClearColor(0.45f, 0.4f, 0.4f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Render();
		m_wgl.Present();
	}

	ImGui_InvalidateDeviceObjects();

	return 0;
}
