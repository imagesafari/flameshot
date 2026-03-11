// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: Yurii Puchkov & Contributors
//

#include "imguploadermanager.h"
#include "src/utils/confighandler.h"
#include <QPixmap>
#include <QWidget>

#include "storages/imgur/imguruploader.h"
#include "storages/screenshotty/screenshottyuploader.h"

ImgUploaderManager::ImgUploaderManager(QObject* parent)
  : QObject(parent)
  , m_imgUploaderBase(nullptr)
{
    init();
}

void ImgUploaderManager::init()
{
    ConfigHandler config;
    m_imgUploaderPlugin = config.uploadStorage();
    if (m_imgUploaderPlugin.isEmpty()) {
        m_imgUploaderPlugin = IMG_UPLOADER_STORAGE_DEFAULT;
    }

    if (m_imgUploaderPlugin == "screenshotty") {
        m_urlString = config.screenshottyServerUrl();
    } else {
        m_urlString = "https://imgur.com/";
    }
}

ImgUploaderBase* ImgUploaderManager::uploader(const QPixmap& capture,
                                              QWidget* parent)
{
    if (m_imgUploaderPlugin == "screenshotty") {
        m_imgUploaderBase =
          new ScreenshottyUploader(capture, parent);
    } else {
        m_imgUploaderBase =
          new ImgurUploader(capture, parent);
    }

    if (m_imgUploaderBase && !capture.isNull()) {
        m_imgUploaderBase->upload();
    }
    return m_imgUploaderBase;
}

ImgUploaderBase* ImgUploaderManager::uploader(const QString& imgUploaderPlugin)
{
    m_imgUploaderPlugin = imgUploaderPlugin;
    init();
    return uploader(QPixmap());
}

const QString& ImgUploaderManager::uploaderPlugin()
{
    return m_imgUploaderPlugin;
}

const QString& ImgUploaderManager::url()
{
    return m_urlString;
}
