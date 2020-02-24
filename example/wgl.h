#pragma once
#include <Windows.h>

class WGL
{
    HDC m_hdc = NULL;
    HGLRC m_hrc = NULL;

public:
    bool Initialize(HWND hwnd)
    {
        m_hdc = GetDC(hwnd);
        if (!m_hdc)
        {
            return false;
        }

        PIXELFORMATDESCRIPTOR pfd =
            {
                sizeof(PIXELFORMATDESCRIPTOR),
                1,
                PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
                PFD_TYPE_RGBA,
                32,
                0, 0, 0, 0, 0, 0, 8, 0,
                0, 0, 0, 0, 0, // accum
                32,            // zbuffer
                0,             // stencil!
                0,             // aux
                PFD_MAIN_PLANE,
                0, 0, 0, 0};

        unsigned int PixelFormat = ChoosePixelFormat(m_hdc, &pfd);
        if (!PixelFormat)
        {
            return false;
        }
        if (!SetPixelFormat(m_hdc, PixelFormat, &pfd))
        {
            return false;
        }
        m_hrc = wglCreateContext(m_hdc);
        if (!m_hrc)
        {
            return false;
        }
        if (!wglMakeCurrent(m_hdc, m_hrc))
        {
            return false;
        }

        return true;
    }

    void Present()
    {
        SwapBuffers(m_hdc);
    }
};
