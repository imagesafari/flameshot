// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 imagesafari Contributors

#pragma once

#include <QList>
#include <QPoint>
#include <QRect>
#include <QString>

struct DetectedWindow
{
    QRect rect;
    QString title;
};

class WindowDetector
{
public:
    WindowDetector();

    // Enumerate all visible, non-minimized top-level windows
    void refresh();

    // Find the smallest window containing the given point (in widget coords).
    // widgetOrigin is the global top-left of the capture widget.
    // Returns an invalid QRect if nothing found.
    QRect windowAt(const QPoint& widgetPos, const QPoint& widgetOrigin) const;

    const QList<DetectedWindow>& windows() const { return m_windows; }

private:
    QList<DetectedWindow> m_windows;
};
