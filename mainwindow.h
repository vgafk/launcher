#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "fileversion.h"
#include "fileupdater.h"

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
    void saveServer(QUrl url);
    void runExecutable();

private:
    Ui::MainWindow *ui;

    FileUpdater *m_updater;

    QVector<QUrl> getServers();
    QUrl getServer(QString host, int port);

    const QString m_defExecutable;

    void saveExecutableFileName(QString fileName);

};
#endif // MAINWINDOW_H
