#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMap>
#include <QSqlDatabase>
#include <QFile>
#include <QUrl>

#include "fileversion.h"

using VersionList = QMap<QString, FileVersion*>;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    bool checkUpdate();
    bool runExecutable();
    bool setBase();

private:
    Ui::MainWindow *ui;

    enum Server{
        Local,
        Remote
    };

    QSqlDatabase m_base;
    const QString m_defExecutable;

    bool connectToBase(QUrl server);
    QUrl serverAddress(Server server);
    void saveServer(QString host, int port);
    void saveExecutableFileName(QString fileName);
    VersionList baseFileList();
    VersionList localFileList();
    bool checkLockalVersion(VersionList);
    bool updateFile(QString fileName);
    bool downloadFile(QFile &f);
    void updateVersion(const QString &name, const FileVersion *version) const;
};
#endif // MAINWINDOW_H
