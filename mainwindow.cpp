#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "yandexworker.h"

#include <QSettings>
#include <QSqlQuery>
#include <QSqlError>
#include <QProcess>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    ,m_defExecutable("teacher_plan.exe")
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::setBase()
{
    m_base = QSqlDatabase::addDatabase("QMYSQL");
    m_base.setDatabaseName(DB_NAME);
    m_base.setUserName(DB_LOGIN);
    m_base.setPassword(DB_PASSWORD);
    if(!connectToBase(serverAddress(Local))){
        if(!connectToBase(serverAddress(Remote))){
            ui->lbl_info->setText("Не могу подключится к серверу, проверте подключение к сети");
            ui->lbl_error->setText(m_base.lastError().text());
            return false;
        }
    }
    saveServer(m_base.hostName(), m_base.port());
    saveExecutableFileName(m_defExecutable);
    return true;
}

bool MainWindow::connectToBase(QUrl server)
{
    m_base.setHostName(server.host());
    m_base.setPort(server.port());
    return m_base.open();
}

QUrl MainWindow::serverAddress(Server server)
{
    QUrl url;
    QSettings s("settings.ini", QSettings::IniFormat);
    switch (server) {
    case Remote:
        url.setHost(s.value("remoteHost", DB_HOST_REMOTE).toString());
        url.setPort(s.value("remotePort", DB_PORT_REMOTE).toInt());
    default:
        url.setHost(s.value("localHost", DB_HOST_LOCAL).toString());
        url.setPort(s.value("localPort", DB_PORT_LOCAL).toInt());
    }
    return url;
}

void MainWindow::saveServer(QString host, int port)
{
     QSettings s("settings.ini", QSettings::IniFormat);
     s.setValue("dbHost", host);
     s.setValue("dbPort", port);
}

void MainWindow::saveExecutableFileName(QString fileName)
{
    QSettings s("settings.ini", QSettings::IniFormat);
    s.setValue("executable", fileName);
}

QMap<QString, FileVersion*> MainWindow::baseFileList()
{
    ui->lbl_info->setText("Проверка наличия новой версии");
    QSqlQuery query;
    QMap<QString, FileVersion*> files;
    query.exec("SELECT file_name, version_major, version_submajor, version_minor, version_subminor "
               "FROM files_update ");

    if(query.lastError().isValid()){
        ui->lbl_info->setText("Ошибка проверки новой версии");
        ui->lbl_error->setText(query.lastError().text());
        return files;
    }

    while (query.next()) {
        files.insert(query.value("file_name").toString(), new FileVersion{
                         query.value("version_major").toInt(),
                         query.value("version_submajor").toInt(),
                         query.value("version_minor").toInt(),
                         query.value("version_subminor").toInt(),
                         this
                     });
    }

    return files;
}

VersionList MainWindow::localFileList()
{
    QMap<QString, FileVersion*> fileVersions;
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("file_version");
    QStringList files = settings.childKeys();
    for(const auto &file: files){
        QString version = settings.value(file).toString();
        fileVersions.insert(file, FileVersion::fromString(version, this));
    }
    return fileVersions;
}

bool MainWindow::checkUpdate()
{
    const auto baseFiles = baseFileList();
    const auto localFiles = localFileList();

    if(baseFiles.isEmpty())
        return false;

    const auto fileNames = baseFiles.keys();

    for(const auto &bf : fileNames){
        auto baseFileVersion = baseFiles.value(bf);
        if(localFiles.value(bf) != baseFileVersion){
            if(updateFile(bf))
                updateVersion(bf, baseFileVersion);
            else
                return false;
        }
    }
    return true;
}

bool MainWindow::updateFile(QString fileName)
{
    QFile f = QFile(fileName);
    f.remove();

    downloadFile(f);

    return true;
}

bool MainWindow::downloadFile(QFile &f)
{
    ui->lbl_info->setText("Скачивание новой версии");

    YandexWorker client(OAUTH_TOKEN);

    // client.uploadFile("C:/local/file.txt", "/remote/path/file.txt");

    // client.downloadFile("https://disk.yandex.ru/d/lNLw9gT6VM1YmQ", "downloaded.exe");


    QSqlQuery query;
    query.exec(QString("SELECT file FROM files_update WHERE file_name = '%1'").arg(f.fileName()));

    if(query.lastError().isValid()){
        ui->lbl_info->setText("Ошибка скачивание новой версии");
        ui->lbl_info->setText(query.lastError().text());
        return false;
    }

    query.next();

    if(!f.open(QIODevice::WriteOnly))
        return false;

    f.write(query.value("file").toByteArray());
    f.close();
    return true;
}

void MainWindow::updateVersion(const QString &name, const FileVersion* version) const
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("file_version");
    settings.setValue(name, version->toString());
}

bool MainWindow::runExecutable()
{
    ui->lbl_info->setText("Запуск...");
    QSettings settings("settings.ini", QSettings::IniFormat);
    auto file = settings.value("executable", m_defExecutable).toString();
    if(QProcess::startDetached(file, QStringList()))
        return true;
    else
        ui->lbl_error->setText("Ошибка запуска исполняемого файла");
        return false;
}

