#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "fileversion.h"
#include "fileupdater.h"
#include "types.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateFiles();

private slots:
    void addLog(QString msg, bool error);
    void saveServer(ServerType type);
    void runExecutable();

private:
    Ui::MainWindow *ui;

    FileUpdater *m_updater;
    const QString m_defExecutable;
    QHash<ServerType, QUrl> m_servers;

    QVector<QUrl> getServers();
    QUrl getServer(QString host, int port);
    void saveExecutableFileName(QString fileName);

};
#endif // MAINWINDOW_H
