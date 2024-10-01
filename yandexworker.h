#ifndef YANDEXWORKER_H
#define YANDEXWORKER_H

#include <QObject>
#include <QNetworkAccessManager>


class YandexWorker : public QObject
{
    Q_OBJECT

public:
    explicit YandexWorker(const QString &token, QObject *parent = nullptr);

    void downloadFile(const QString& yandexDiskPath, const QString& localFilePath);

signals:

private:
    const QString m_token;
    QNetworkAccessManager* m_networkManager;

};

#endif // YANDEXWORKER_H
