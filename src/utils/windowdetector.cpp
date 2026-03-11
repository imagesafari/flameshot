// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 imagesafari Contributors

#include "windowdetector.h"
#include <QtGlobal>

#if defined(Q_OS_WIN)
#include <dwmapi.h>
#include <windows.h>
#endif

WindowDetector::WindowDetector() {}

#if defined(Q_OS_WIN)

struct EnumContext
{
    QList<DetectedWindow>* windows;
};

static BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam)
{
    auto* ctx = reinterpret_cast<EnumContext*>(lParam);

    if (!IsWindowVisible(hwnd)) {
        return TRUE;
    }
    if (IsIconic(hwnd)) {
        return TRUE;
    }

    // Skip windows with no title (tooltips, etc.)
    int len = GetWindowTextLengthW(hwnd);
    if (len == 0) {
        return TRUE;
    }

    // Skip windows with WS_EX_TOOLWINDOW (floating toolbars, etc.)
    LONG exStyle = GetWindowLongW(hwnd, GWL_EXSTYLE);
    if (exStyle & WS_EX_TOOLWINDOW) {
        return TRUE;
    }

    // Try to get the actual visible rect via DWM (handles Aero snap, etc.)
    RECT winRect;
    HRESULT hr = DwmGetWindowAttribute(
      hwnd, DWMWA_EXTENDED_FRAME_BOUNDS, &winRect, sizeof(winRect));
    if (FAILED(hr)) {
        if (!GetWindowRect(hwnd, &winRect)) {
            return TRUE;
        }
    }

    int w = winRect.right - winRect.left;
    int h = winRect.bottom - winRect.top;
    if (w <= 0 || h <= 0) {
        return TRUE;
    }

    // Skip tiny windows (< 50px in either dimension)
    if (w < 50 || h < 50) {
        return TRUE;
    }

    // Get window title
    std::wstring titleBuf(len + 1, L'\0');
    GetWindowTextW(hwnd, titleBuf.data(), len + 1);

    DetectedWindow dw;
    dw.rect = QRect(winRect.left, winRect.top, w, h);
    dw.title = QString::fromWCharArray(titleBuf.c_str());
    ctx->windows->append(dw);

    return TRUE;
}

void WindowDetector::refresh()
{
    m_windows.clear();
    EnumContext ctx;
    ctx.windows = &m_windows;
    EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&ctx));
}

QRect WindowDetector::windowAt(const QPoint& widgetPos,
                               const QPoint& widgetOrigin) const
{
    // Convert widget coordinates to global screen coordinates
    QPoint globalPos = widgetPos + widgetOrigin;

    QRect best;
    int bestArea = INT_MAX;

    for (const auto& dw : m_windows) {
        if (dw.rect.contains(globalPos)) {
            int area = dw.rect.width() * dw.rect.height();
            if (area < bestArea) {
                bestArea = area;
                best = dw.rect;
            }
        }
    }

    if (best.isValid()) {
        // Convert from global screen coords to widget coords
        return best.translated(-widgetOrigin);
    }
    return QRect();
}

#else
// Stub implementations for non-Windows platforms
void WindowDetector::refresh()
{
    m_windows.clear();
}

QRect WindowDetector::windowAt(const QPoint& widgetPos,
                               const QPoint& widgetOrigin) const
{
    Q_UNUSED(widgetPos)
    Q_UNUSED(widgetOrigin)
    return QRect();
}
#endif
