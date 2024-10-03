#ifndef FILEUPDATER_H
#define FILEUPDATER_H

#include <QHash>
#include <QObject>
#include <QThread>
#include <QUrl>
#include <QNetworkAccessManager>

#include "fileversion.h"
#include "types.h"

class QNetworkReply;

using RemoteList = QMap<int, FileVersion*>;
using LocalList = QMap<QString, FileVersion*>;

class FileUpdater : public QThread
{
    Q_OBJECT
public:
    explicit FileUpdater(QMap<ServerType, QUrl> servers, QObject *parent = nullptr);
    ~FileUpdater() {}

    void run() override;

signals:
    void message(QString, bool);
    void serverChanged(ServerType);
    void complited();

private:
    QMap<ServerType, QUrl> m_serverList;

    enum Endpoint{
        FileList,
        Download,
        Check
    };

    QHash<Endpoint, QString> m_endpoints;
    QList<QNetworkReply*> m_replyes;
    QUrl m_currentDatabase;
    QNetworkAccessManager *m_manager;

    void selectDatabase();
    bool checkDatabase(const QUrl &url);
    LocalList localFileList();
    void checkFiles();
    void checkComplited();
    void compareFiles(LocalList local, RemoteList remote);
    void saveFile();
    void updateVersion(const FileVersion* version) const;
};

#endif // FILEUPDATER_H
