#pragma once
#include <stdint.h>

namespace screenstate
{
enum MouseButtonFlags
{
    None,
    LeftDown = 0x01,
    RightDown = 0x02,
    MiddleDown = 0x04,
    WheelPlus = 0x08,
    WheelMinus = 0x10,
    CursorUpdate = 0x20,
};

struct ScreenState
{
    uint32_t Time;
    MouseButtonFlags MouseFlag;
    int16_t Width;
    int16_t Height;
    int16_t X;
    int16_t Y;

    bool Has(MouseButtonFlags flag) const
    {
        return (MouseFlag & flag) != 0;
    }

    void Set(MouseButtonFlags flag)
    {
        MouseFlag = (MouseButtonFlags)(MouseFlag | flag);
    }

    void Unset(MouseButtonFlags flag)
    {
        MouseFlag = (MouseButtonFlags)(MouseFlag & ~flag);
    }

    void Clear()
    {
        Unset((MouseButtonFlags)(MouseButtonFlags::WheelMinus | MouseButtonFlags::WheelPlus | MouseButtonFlags::CursorUpdate));
    }

    float AspectRatio() const
    {
        if (Height == 0)
        {
            return 1;
        }
        return (float)Width / Height;
    }

    bool HasCapture() const
    {
        return Has(MouseButtonFlags::LeftDown) || Has(MouseButtonFlags::RightDown) || Has(MouseButtonFlags::MiddleDown);
    }

    bool SameSize(const ScreenState &rhs) const
    {
        return Width == rhs.Width && Height == rhs.Height;
    }

    float DeltaSeconds(const ScreenState &rhs) const
    {
        return 0.001f * (Time - rhs.Time);
    }
};
static_assert(sizeof(ScreenState) == 16, "sizeof(WindowMouseState)");
} // namespace screenstate
