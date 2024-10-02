#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QSqlDatabase>
#include <QFile>
#include <QUrl>
#include <QNetworkAccessManager>

#include "fileversion.h"

using RemoteList = QMap<int, FileVersion*>;
using LocalList = QMap<QString, FileVersion*>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool checkSqlBase();
    bool runExecutable();
    void checkFiles();

private slots:
    void checkUpdate(QNetworkReply *reply);
    void saveFile(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;

    enum Server{
        Local,
        Remote
    };

    enum Endpoint{
        List,
        Download
    };

    QSqlDatabase m_base;
    const QString m_defExecutable;
    const QString m_localFileListEndpoint;
    const QString m_localDownloadEndpoint;
    const QString m_internalFileListEndpoint;
    const QString m_internalDownloadEndpoint;

    QNetworkAccessManager *m_manager;
    Server m_currentServer;

    bool connectToBase(QUrl server);
    QUrl serverAddress(Server server);
    void saveServer(QString host, int port);
    void saveExecutableFileName(QString fileName);
    LocalList localFileList();
    void updateVersion(const FileVersion *version) const;
    void compareFiles(LocalList local, RemoteList remote);
    const QString &getEndpoint(Server server, Endpoint endpoint);
};
#endif // MAINWINDOW_H
