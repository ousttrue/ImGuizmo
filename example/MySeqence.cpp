#include "MySequence.h"
#include <imgui_internal.h>
#include <ImSequencer.h>

static const char *SequencerItemTypeNames[] = {"Camera", "Music", "ScreenEffect", "FadeIn", "Animation"};

static inline ImVec2 operator-(const ImVec2 &lhs, const ImVec2 &rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

int MySequence::GetFrameMin() const
{
    return mFrameMin;
}
int MySequence::GetFrameMax() const
{
    return mFrameMax;
}
int MySequence::GetItemCount() const { return (int)myItems.size(); }

int MySequence::GetItemTypeCount() const { return sizeof(SequencerItemTypeNames) / sizeof(char *); }
const char *MySequence::GetItemTypeName(int typeIndex) const { return SequencerItemTypeNames[typeIndex]; }
const char *MySequence::GetItemLabel(int index) const
{
    static char tmps[512];
    sprintf_s(tmps, "[%02d] %s", index, SequencerItemTypeNames[myItems[index].mType]);
    return tmps;
}

void MySequence::Get(int index, int **start, int **end, int *type, unsigned int *color)
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
void MySequence::Add(int type) { myItems.push_back(MySequenceItem{type, 0, 10, false}); };
void MySequence::Del(int index) { myItems.erase(myItems.begin() + index); }
void MySequence::Duplicate(int index) { myItems.push_back(myItems[index]); }

size_t MySequence::GetCustomHeight(int index) { return myItems[index].mExpanded ? 300 : 0; }

void MySequence::DoubleClick(int index)
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

void MySequence::CustomDraw(int index, ImDrawList *draw_list, const ImRect &rc, const ImRect &legendRect, const ImRect &clippingRect, const ImRect &legendClippingRect)
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

void MySequence::CustomDrawCompact(int index, ImDrawList *draw_list, const ImRect &rc, const ImRect &clippingRect)
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

void MySequence::Gui()
{
    ImGui::Begin("Sequencer");

    ImGui::PushItemWidth(130);
    ImGui::InputInt("Frame Min", &mFrameMin);
    ImGui::SameLine();
    ImGui::InputInt("Frame ", &currentFrame);
    ImGui::SameLine();
    ImGui::InputInt("Frame Max", &mFrameMax);
    ImGui::PopItemWidth();
    Sequencer(this, &currentFrame, &expanded, &selectedEntry, &firstFrame, ImSequencer::SEQUENCER_EDIT_STARTEND | ImSequencer::SEQUENCER_ADD | ImSequencer::SEQUENCER_DEL | ImSequencer::SEQUENCER_COPYPASTE | ImSequencer::SEQUENCER_CHANGE_FRAME);
    // add a UI to edit that particular item
    if (selectedEntry != -1)
    {
        const MySequence::MySequenceItem &item = myItems[selectedEntry];
        ImGui::Text("I am a %s, please edit me", SequencerItemTypeNames[item.mType]);
        // switch (type) ....
    }

    ImGui::End();
}
