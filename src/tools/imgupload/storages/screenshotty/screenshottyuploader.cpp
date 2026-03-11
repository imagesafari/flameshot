// SPDX-License-Identifier: GPL-3.0-or-later
// SPDX-FileCopyrightText: 2024 imagesafari Contributors

#include "screenshottyuploader.h"
#include "src/utils/confighandler.h"
#include "src/utils/history.h"
#include "src/widgets/loadspinner.h"
#include "src/widgets/notificationwidget.h"
#include <QBuffer>
#include <QHttpMultiPart>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QShortcut>

ScreenshottyUploader::ScreenshottyUploader(const QPixmap& capture,
                                           QWidget* parent)
  : ImgUploaderBase(capture, parent)
{
    m_NetworkAM = new QNetworkAccessManager(this);
    connect(m_NetworkAM,
            &QNetworkAccessManager::finished,
            this,
            &ScreenshottyUploader::handleReply);
}

void ScreenshottyUploader::handleReply(QNetworkReply* reply)
{
    spinner()->deleteLater();
    m_currentImageName.clear();

    QJsonDocument response = QJsonDocument::fromJson(reply->readAll());
    QJsonObject json = response.object();

    if (reply->error() == QNetworkReply::NoError) {
        QString url = json[QStringLiteral("url")].toString();
        if (url.isEmpty()) {
            url = json[QStringLiteral("direct_url")].toString();
        }
        setImageURL(QUrl(url));

        // Use the shortcode as a delete token (server manages deletion)
        QString shortcode = url;
        int lastSlash = shortcode.lastIndexOf("/");
        if (lastSlash >= 0) {
            shortcode = shortcode.mid(lastSlash + 1);
        }
        // Remove file extension if present
        int dot = shortcode.lastIndexOf(".");
        if (dot >= 0) {
            shortcode = shortcode.left(dot);
        }

        // Save to history
        History history;
        m_currentImageName =
          history.packFileName("screenshotty", shortcode, shortcode + ".png");
        history.save(pixmap(), m_currentImageName);

        emit uploadOk(imageURL());
    } else {
        QString errorMsg = json[QStringLiteral("error")].toString();
        if (errorMsg.isEmpty()) {
            errorMsg = reply->errorString();
        }
        setInfoLabelText(errorMsg);
    }

    new QShortcut(Qt::Key_Escape, this, SLOT(close()));
}

void ScreenshottyUploader::upload()
{
    ConfigHandler config;
    QString serverUrl = config.screenshottyServerUrl();
    QString apiKey = config.screenshottyApiKey();

    if (serverUrl.isEmpty()) {
        setInfoLabelText(
          tr("Screenshotty server URL is not configured.\n"
             "Please set it in Settings > General."));
        return;
    }

    QByteArray byteArray;
    QBuffer buffer(&byteArray);
    buffer.open(QIODevice::WriteOnly);
    pixmap().save(&buffer, "PNG");

    auto* multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart imagePart;
    imagePart.setHeader(QNetworkRequest::ContentTypeHeader,
                        QVariant("image/png"));
    imagePart.setHeader(
      QNetworkRequest::ContentDispositionHeader,
      QVariant("form-data; name=\"file\"; filename=\"screenshot.png\""));
    imagePart.setBody(byteArray);
    multiPart->append(imagePart);

    // Ensure URL ends with /api/upload
    if (!serverUrl.endsWith("/")) {
        serverUrl += "/";
    }
    QUrl url(serverUrl + "api/upload");
    QNetworkRequest request(url);

    if (!apiKey.isEmpty()) {
        request.setRawHeader(
          "Authorization",
          QStringLiteral("Bearer %1").arg(apiKey).toUtf8());
    }

    QNetworkReply* reply = m_NetworkAM->post(request, multiPart);
    multiPart->setParent(reply);
}

void ScreenshottyUploader::deleteImage(const QString& fileName,
                                       const QString& deleteToken)
{
    Q_UNUSED(fileName)
    Q_UNUSED(deleteToken)
    // Screenshotty server manages image deletion through the admin panel.
    // No client-side deletion API is currently available.
    notification()->showMessage(
      tr("Delete images from the Screenshotty admin panel."));
    emit deleteOk();
}
