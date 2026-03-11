// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 imagesafari Contributors

#pragma once

#include "src/tools/imgupload/storages/imguploaderbase.h"
#include <QUrl>
#include <QWidget>

class QNetworkReply;
class QNetworkAccessManager;

class ScreenshottyUploader : public ImgUploaderBase
{
    Q_OBJECT
public:
    explicit ScreenshottyUploader(const QPixmap& capture,
                                  QWidget* parent = nullptr);
    void deleteImage(const QString& fileName, const QString& deleteToken);

private slots:
    void handleReply(QNetworkReply* reply);

private:
    void upload();

private:
    QNetworkAccessManager* m_NetworkAM;
};
