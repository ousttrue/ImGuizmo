#include "camera.h"
#include "imgui.h"
#define IMAPP_IMPL
#include "glutil.h"
#include "Win32Window.h"
#include "ImGuiImplScreenState.h"
#include "Renderer.h"
#include "wgl.h"


#include "ImGuizmo.h"
#include "ImSequencer.h"

#include <math.h>
#include <vector>
#include <algorithm>
#include "ImCurveEdit.h"
#include "imgui_internal.h"
//
//
// ImGuizmo example helper functions
//
//
static inline ImVec2 operator-(const ImVec2 &lhs, const ImVec2 &rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

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

//
//
// ImSequencer interface
//
//
static const char *SequencerItemTypeNames[] = {"Camera", "Music", "ScreenEffect", "FadeIn", "Animation"};

struct RampEdit : public ImCurveEdit::Delegate
{
	RampEdit()
	{
		mPts[0][0] = ImVec2(-10.f, 0);
		mPts[0][1] = ImVec2(20.f, 0.6f);
		mPts[0][2] = ImVec2(25.f, 0.2f);
		mPts[0][3] = ImVec2(70.f, 0.4f);
		mPts[0][4] = ImVec2(120.f, 1.f);
		mPointCount[0] = 5;

		mPts[1][0] = ImVec2(-50.f, 0.2f);
		mPts[1][1] = ImVec2(33.f, 0.7f);
		mPts[1][2] = ImVec2(80.f, 0.2f);
		mPts[1][3] = ImVec2(82.f, 0.8f);
		mPointCount[1] = 4;

		mPts[2][0] = ImVec2(40.f, 0);
		mPts[2][1] = ImVec2(60.f, 0.1f);
		mPts[2][2] = ImVec2(90.f, 0.82f);
		mPts[2][3] = ImVec2(150.f, 0.24f);
		mPts[2][4] = ImVec2(200.f, 0.34f);
		mPts[2][5] = ImVec2(250.f, 0.12f);
		mPointCount[2] = 6;
		mbVisible[0] = mbVisible[1] = mbVisible[2] = true;
		mMax = ImVec2(1.f, 1.f);
		mMin = ImVec2(0.f, 0.f);
	}
	size_t GetCurveCount()
	{
		return 3;
	}

	bool IsVisible(size_t curveIndex)
	{
		return mbVisible[curveIndex];
	}
	size_t GetPointCount(size_t curveIndex)
	{
		return mPointCount[curveIndex];
	}

	uint32_t GetCurveColor(size_t curveIndex)
	{
		uint32_t cols[] = {0xFF0000FF, 0xFF00FF00, 0xFFFF0000};
		return cols[curveIndex];
	}
	ImVec2 *GetPoints(size_t curveIndex)
	{
		return mPts[curveIndex];
	}
	virtual ImCurveEdit::CurveType GetCurveType(size_t curveIndex) const { return ImCurveEdit::CurveSmooth; }
	virtual int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value)
	{
		mPts[curveIndex][pointIndex] = ImVec2(value.x, value.y);
		SortValues(curveIndex);
		for (size_t i = 0; i < GetPointCount(curveIndex); i++)
		{
			if (mPts[curveIndex][i].x == value.x)
				return (int)i;
		}
		return pointIndex;
	}
	virtual void AddPoint(size_t curveIndex, ImVec2 value)
	{
		if (mPointCount[curveIndex] >= 8)
			return;
		mPts[curveIndex][mPointCount[curveIndex]++] = value;
		SortValues(curveIndex);
	}
	virtual ImVec2 &GetMax() { return mMax; }
	virtual ImVec2 &GetMin() { return mMin; }
	virtual unsigned int GetBackgroundColor() { return 0; }
	ImVec2 mPts[3][8];
	size_t mPointCount[3];
	bool mbVisible[3];
	ImVec2 mMin;
	ImVec2 mMax;

private:
	void SortValues(size_t curveIndex)
	{
		auto b = std::begin(mPts[curveIndex]);
		auto e = std::begin(mPts[curveIndex]) + GetPointCount(curveIndex);
		std::sort(b, e, [](ImVec2 a, ImVec2 b) { return a.x < b.x; });
	}
};

struct MySequence : public ImSequencer::SequenceInterface
{
	// interface with sequencer

	virtual int GetFrameMin() const
	{
		return mFrameMin;
	}
	virtual int GetFrameMax() const
	{
		return mFrameMax;
	}
	virtual int GetItemCount() const { return (int)myItems.size(); }

	virtual int GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char *); }
	virtual const char *GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
	virtual const char *GetItemLabel(int index) const
	{
		static char tmps[512];
		sprintf_s(tmps, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
		return tmps;
	}

	virtual void Get(int index, int **start, int **end, int *type, unsigned int *color)
	{
		MySequenceItem &item = myItems[index];
		if (color)
			*color = 0xFFAA8080; // same color for everyone, return color based on type
		if (start)
			*start = &item.mFrameStart;
		if (end)
			*end = &item.mFrameEnd;
		if (type)
			*type = item.mType;
	}
	virtual void Add(int type) { myItems.push_back(MySequenceItem{type, 0, 10, false}); };
	virtual void Del(int index) { myItems.erase(myItems.begin() + index); }
	virtual void Duplicate(int index) { myItems.push_back(myItems[index]); }

	virtual size_t GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

	// my datas
	MySequence() : mFrameMin(0), mFrameMax(0) {}
	int mFrameMin, mFrameMax;
	struct MySequenceItem
	{
		int mType;
		int mFrameStart, mFrameEnd;
		bool mExpanded;
	};
	std::vector<MySequenceItem> myItems;
	RampEdit rampEdit;

	virtual void DoubleClick(int index)
	{
		if (myItems[index].mExpanded)
		{
			myItems[index].mExpanded = false;
			return;
		}
		for (auto &item : myItems)
			item.mExpanded = false;
		myItems[index].mExpanded = !myItems[index].mExpanded;
	}

	virtual void CustomDraw(int index, ImDrawList *draw_list, const ImRect &rc, const ImRect &legendRect, const ImRect &clippingRect, const ImRect &legendClippingRect)
	{
		static const char *labels[] = {"Translation", "Rotation", "Scale"};

		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(legendClippingRect.Min, legendClippingRect.Max, true);
		for (int i = 0; i < 3; i++)
		{
			ImVec2 pta(legendRect.Min.x + 30, legendRect.Min.y + i * 14.f);
			ImVec2 ptb(legendRect.Max.x, legendRect.Min.y + (i + 1) * 14.f);
			draw_list->AddText(pta, rampEdit.mbVisible[i] ? 0xFFFFFFFF : 0x80FFFFFF, labels[i]);
			if (ImRect(pta, ptb).Contains(ImGui::GetMousePos()) && ImGui::IsMouseClicked(0))
				rampEdit.mbVisible[i] = !rampEdit.mbVisible[i];
		}
		draw_list->PopClipRect();

		ImGui::SetCursorScreenPos(rc.Min);
		ImCurveEdit::Edit(rampEdit, rc.Max - rc.Min, 137 + index, &clippingRect);
	}

	virtual void CustomDrawCompact(int index, ImDrawList *draw_list, const ImRect &rc, const ImRect &clippingRect)
	{
		rampEdit.mMax = ImVec2(float(mFrameMax), 1.f);
		rampEdit.mMin = ImVec2(float(mFrameMin), 0.f);
		draw_list->PushClipRect(clippingRect.Min, clippingRect.Max, true);
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < rampEdit.mPointCount[i]; j++)
			{
				float p = rampEdit.mPts[i][j].x;
				if (p < myItems[index].mFrameStart || p > myItems[index].mFrameEnd)
					continue;
				float r = (p - mFrameMin) / float(mFrameMax - mFrameMin);
				float x = ImLerp(rc.Min.x, rc.Max.x, r);
				draw_list->AddLine(ImVec2(x, rc.Min.y + 6), ImVec2(x, rc.Max.y - 4), 0xAA000000, 4.f);
			}
		}
		draw_list->PopClipRect();
	}
};

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

		// ImGui::SetNextWindowPos(ImVec2(1024, 100));
		// ImGui::SetNextWindowSize(ImVec2(256, 256));

		// create a window and insert the inspector
		// ImGui::SetNextWindowPos(ImVec2(10, 10));
		// ImGui::SetNextWindowSize(ImVec2(320, 340));
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
		static int selectedEntry = -1;
		static int firstFrame = 0;
		static bool expanded = true;
		static int currentFrame = 100;
		// ImGui::SetNextWindowPos(ImVec2(10, 350));

		// ImGui::SetNextWindowSize(ImVec2(940, 480));
		ImGui::Begin("Sequencer");

		ImGui::PushItemWidth(130);
		ImGui::InputInt("Frame Min", &mySequence.mFrameMin);
		ImGui::SameLine();
		ImGui::InputInt("Frame ", &currentFrame);
		ImGui::SameLine();
		ImGui::InputInt("Frame Max", &mySequence.mFrameMax);
		ImGui::PopItemWidth();
		Sequencer(&mySequence, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
		// add a UI to edit that particular item
		if (selectedEntry != -1)
		{
			const MySequence::MySequenceItem &item = mySequence.myItems[selectedEntry];
			ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
			// switch (type) ....
		}

		ImGui::End();

		ImGuizmo::ViewManipulate(camera.cameraView, camera.camDistance, ImVec2(io.DisplaySize.x - 128, 0), ImVec2(128, 128), 0x10101010);

		// render everything
		glClearColor(0.45f, 0.4f, 0.4f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);

		ImGui::Render();
		// ImGui::Render();
		// imApp.EndFrame();
		m_wgl.Present();
	}

	ImGui_InvalidateDeviceObjects();

	return 0;
}
