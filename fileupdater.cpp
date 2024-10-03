#include "fileupdater.h"

#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QFile>

#define TIMER_TIMEUOT 5000

FileUpdater::FileUpdater(QMap<ServerType, QUrl> servers, QObject *parent)
    : QThread{parent}
    , m_manager(new QNetworkAccessManager)
{
    m_serverList.insert(servers);

    m_endpoints.insert(FileList, "/files");
    m_endpoints.insert(Download, "/download");
    m_endpoints.insert(Check, "/check");
}

void FileUpdater::run()
{
    selectDatabase();
    checkFiles();
}

void FileUpdater::selectDatabase()
{
    for(auto it = m_serverList.begin(); it != m_serverList.end(); ++it){
        auto type = it.key();
        auto url = it.value();
        if(checkDatabase(url)){
            emit serverChanged(type);
            emit message(QString("Выбран сервер %1").arg(url.host()), false);
            return;
        }
    }

    emit serverChanged(ServerType::Empty);
    emit message("Серверы не доступны, обратитесь в компьютерный центр", true);
}

bool FileUpdater::checkDatabase(const QUrl &url)
{
    QUrl endpoint = url;
    endpoint.setPath("/check");
    emit message(QString("Проверка подключения к серверу %1...").arg(endpoint.host()), false);

    QEventLoop loop;
    QNetworkReply *reply = m_manager->get(QNetworkRequest(endpoint));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    QTimer timer;
    timer.setSingleShot(true);
    QObject::connect(&timer, &QTimer::timeout, this, [&]() {
        emit message("Время ожидания истекло. Прерывание запроса.", true);
        reply->abort();
        loop.quit();
        return false;
    });
    timer.start(TIMER_TIMEUOT);
    loop.exec();

    bool avalible;
    if (reply->error() == QNetworkReply::NoError){
        m_currentDatabase = url;
        avalible = true;
    } else {
        emit message(reply->errorString(), true);
        avalible = false;
    }
    reply->deleteLater();
    return avalible;
}

LocalList FileUpdater::localFileList()
{
    LocalList fileVersions;
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("file_version");
    QStringList files = settings.childKeys();
    for(const auto &file: files){
        QString version = settings.value(file).toString();
        fileVersions.insert(file, FileVersion::fromString(file, version, this));
    }
    return fileVersions;
}

void FileUpdater::checkFiles()
{
    emit message("Проверка наличия новой версии...", false);

    QEventLoop loop;
    auto endpoint = m_currentDatabase;
    endpoint.setPath(m_endpoints.value(FileList));
    QNetworkReply *reply = m_manager->get(QNetworkRequest(endpoint));
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);

    loop.exec();

    if (reply->error() != QNetworkReply::NoError) {
        emit message(reply->errorString(), true);
        reply->deleteLater();
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonArray filesArray = doc.array();

    RemoteList remoteFiles;
    for (const QJsonValue &value : filesArray) {
        QJsonObject fileObj = value.toObject();
        QString fileName = fileObj["file_name"].toString();
        int id = fileObj["id"].toInt();
        int major = fileObj["version_major"].toInt();
        int submajor = fileObj["version_submajor"].toInt();
        int minor = fileObj["version_minor"].toInt();
        int subminor = fileObj["version_subminor"].toInt();
        remoteFiles.insert(id, new FileVersion(fileName, major, submajor, minor, subminor, this));
    }
    reply->deleteLater();

    compareFiles(localFileList(), remoteFiles);
}

void FileUpdater::checkComplited()
{
    if(m_replyes.isEmpty())
        emit complited();
}

void FileUpdater::compareFiles(LocalList local, RemoteList remote)
{
    auto endpoint = m_currentDatabase;

    for (auto it = remote.begin(); it != remote.end(); ++it) {
        int id = it.key();
        FileVersion *remoteVersion = it.value();
        const QString &fileName = remoteVersion->name();

        if (!local.contains(fileName) || local[fileName] < remoteVersion) {
            endpoint.setPath(m_endpoints.value(Download) + QString("/%1").arg(id));
            auto reply = m_manager->get(QNetworkRequest(QUrl(endpoint)));
            m_replyes.append(reply);
            connect(reply, &QNetworkReply::finished, this, &FileUpdater::saveFile);
        }
    }
}

void FileUpdater::saveFile()
{
    auto reply = qobject_cast<QNetworkReply*>(sender());
    if (reply->error() != QNetworkReply::NoError) {
        emit message(reply->errorString(), true);
        reply->deleteLater();
        return;
    }

    QString fileName = reply->rawHeader("X-File-Name");
    if (fileName.isEmpty()) {
        emit message("Имя файла отсутствует в заголовках.", true);
        reply->deleteLater();
        return;
    }

    int major = reply->rawHeader("X-Version-Major").toInt();
    int submajor = reply->rawHeader("X-Version-Submajor").toInt();
    int minor = reply->rawHeader("X-Version-Minor").toInt();
    int subminor = reply->rawHeader("X-Version-Subminor").toInt();

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        emit message("Не удалось открыть файл для записи:", true);
        reply->deleteLater();
        return;
    }

    file.write(reply->readAll());
    file.close();

    emit message(QString("Файл сохранен как: %1, версия: %2.%3.%4.%5")
                 .arg(fileName).arg(major).arg(submajor).arg(minor).arg(subminor), false);
    updateVersion(new FileVersion(fileName, major, submajor, minor, subminor, this));
    m_replyes.removeOne(reply);
    reply->deleteLater();

    checkComplited();
}

void FileUpdater::updateVersion(const FileVersion *version) const
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("file_version");
    settings.setValue(version->name(), version->toString());
}
