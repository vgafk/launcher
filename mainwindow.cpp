#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QListWidgetItem>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_defExecutable("teacher_plan.exe")
{
    ui->setupUi(this);

    m_updater = new FileUpdater({{Remote, getServer("83.167.69.146", 5807)},
                                 {Local, getServer("10.0.2.18", 8100)}
                                });

    m_servers.insert(Local, getServer("10.0.2.18", 3306));
    m_servers.insert(Remote, getServer("83.167.69.146", 5806));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::updateFiles()
{
    connect(m_updater, &FileUpdater::message, this, &MainWindow::addLog);
    connect(m_updater, &FileUpdater::serverChanged, this, &MainWindow::saveServer);
    connect(m_updater, &FileUpdater::complited, this, &MainWindow::runExecutable);

    m_updater->run();
}

void MainWindow::addLog(QString msg, bool error)
{
    QListWidgetItem *item = new QListWidgetItem(msg, ui->lw_log);
    if(error){
        item->setForeground(Qt::red);
    }
    ui->lw_log->addItem(item);
}

void MainWindow::saveServer(ServerType type)
{
    if(type == ServerType::Empty)
        return;

    auto url = m_servers.value(type);
    QSettings s("settings.ini", QSettings::IniFormat);
    s.setValue("dbHost", url.host());
    s.setValue("dbPort", url.port());
}

QUrl MainWindow::getServer(QString host, int port)
{
    QUrl url;
    url.setHost(host);
    url.setPort(port);
    url.setScheme("http");
    return url;
}

void MainWindow::saveExecutableFileName(QString fileName)
{
    QSettings s("settings.ini", QSettings::IniFormat);
    s.setValue("executable", fileName);
}

void MainWindow::runExecutable()
{
    addLog("Запуск...", false);
    QSettings settings("settings.ini", QSettings::IniFormat);
    auto file = settings.value("executable", m_defExecutable).toString();
    if(QProcess::startDetached(file, QStringList())){
        saveExecutableFileName(file);
        this->close();
    }else{
        addLog("Ошибка запуска исполняемого файла", true);
    }
}
