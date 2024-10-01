#include "yandexworker.h"

#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>

YandexWorker::YandexWorker(const QString &token, QObject *parent):
    QObject(parent)
    , m_token(token)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

void YandexWorker::downloadFile(const QString &yandexDiskPath, const QString &localFilePath) {
    QNetworkRequest request(QUrl("https://cloud-api.yandex.net/v1/disk/resources/download?path=" + yandexDiskPath));
    request.setRawHeader("Authorization", ("OAuth " + m_token).toUtf8());

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, [this, reply, localFilePath]() {
        if (reply->error() == QNetworkReply::NoError) {
            QJsonDocument jsonResponse = QJsonDocument::fromJson(reply->readAll());
            QString downloadUrl = jsonResponse.object().value("href").toString();

            // Скачиваем файл по полученному URL
            QNetworkRequest downloadRequest(downloadUrl);
            QNetworkReply* downloadReply = m_networkManager->get(downloadRequest);
            connect(downloadReply, &QNetworkReply::finished, [downloadReply, localFilePath]() {
                if (downloadReply->error() == QNetworkReply::NoError) {
                    QFile file(localFilePath);
                    if (file.open(QIODevice::WriteOnly)) {
                        file.write(downloadReply->readAll());
                        file.close();
                        qDebug() << "Файл успешно скачан с Яндекс.Диска.";
                    } else {
                        qWarning() << "Не удалось открыть файл для записи:" << localFilePath;
                    }
                } else {
                    qWarning() << "Ошибка при скачивании файла:" << downloadReply->errorString();
                }
                downloadReply->deleteLater();
            });
        } else {
            qWarning() << "Ошибка получения ссылки для скачивания:" << reply->errorString();
        }
        reply->deleteLater();
    });
}
