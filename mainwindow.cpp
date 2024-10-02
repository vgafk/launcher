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

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_defExecutable("teacher_plan.exe")
    , m_manager(new QNetworkAccessManager(this))
    ,m_localFileListEndpoint("http://10.0.2.18:8100/files")
    ,m_localDownloadEndpoint("http://10.0.2.18:8100/download")
    ,m_internalFileListEndpoint("http://83.167.69.146:5807/files")
    ,m_internalDownloadEndpoint("http://83.167.69.146:5807/download")
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::checkSqlBase()
{
    m_base = QSqlDatabase::addDatabase("QMYSQL");
    m_base.setDatabaseName(DB_NAME);
    m_base.setUserName(DB_LOGIN);
    m_base.setPassword(DB_PASSWORD);
    m_currentServer = Local;
    if(!connectToBase(serverAddress(m_currentServer))){
        m_currentServer = Remote;
        if(!connectToBase(serverAddress(m_currentServer))){
            ui->lbl_info->setText("Не могу подключится к серверу, проверте подключение к сети");
            ui->lbl_error->setText(m_base.lastError().text());
            return false;
        }
    }
    saveServer(m_base.hostName(), m_base.port());
    saveExecutableFileName(m_defExecutable);
    return true;
}

void MainWindow::checkFiles()
{
    ui->lbl_info->setText("Проверка наличия новой версии");

    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::checkUpdate);
    m_manager->get(QNetworkRequest(QUrl(getEndpoint(m_currentServer, List))));
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
        break;
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

LocalList MainWindow::localFileList()
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

void MainWindow::checkUpdate(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qWarning() << "Network error:" << reply->errorString();
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

void MainWindow::saveFile(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Ошибка скачивания файла:" << reply->errorString();
        reply->deleteLater();
        return;
    }

    // Извлекаем имя файла из заголовков
    QString fileName = reply->rawHeader("X-File-Name");
    if (fileName.isEmpty()) {
        qDebug() << "Имя файла отсутствует в заголовках.";
        reply->deleteLater();
        return;
    }

    // Извлекаем версии из заголовков
    int major = reply->rawHeader("X-Version-Major").toInt();
    int submajor = reply->rawHeader("X-Version-Submajor").toInt();
    int minor = reply->rawHeader("X-Version-Minor").toInt();
    int subminor = reply->rawHeader("X-Version-Subminor").toInt();

    // Сохраняем файл
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Не удалось открыть файл для записи:" << file.errorString();
        reply->deleteLater();
        return;
    }

    // Записываем данные в файл
    file.write(reply->readAll());
    file.close();

    // Выводим информацию о версиях
    qDebug() << "Файл сохранен как:" << fileName;
    qDebug() << "Версии:" << major << submajor << minor << subminor;

    updateVersion(new FileVersion(fileName, major, submajor, minor, subminor, this));

    // Удаляем объект reply
    reply->deleteLater();
}

void MainWindow::updateVersion(const FileVersion* version) const
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    settings.beginGroup("file_version");
    settings.setValue(version->name(), version->toString());
}

void MainWindow::compareFiles(LocalList local, RemoteList remote)
{
    disconnect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::checkUpdate);

    connect(m_manager, &QNetworkAccessManager::finished, this, &MainWindow::saveFile);

    for (auto it = remote.begin(); it != remote.end(); ++it) {
        int id = it.key();
        FileVersion *remoteVersion = it.value();
        const QString &fileName = remoteVersion->name();

        if (!local.contains(fileName) || local[fileName] < remoteVersion) {
            m_manager->get(QNetworkRequest(QUrl(getEndpoint(m_currentServer, Download) + QString("/%1").arg(id))));
        }
    }
}

const QString &MainWindow::getEndpoint(Server server, Endpoint endpoint)
{
    if(server == Local){
        if(endpoint == List)
            return m_localFileListEndpoint;
        else
            return m_localDownloadEndpoint;
    } else {
        if(endpoint == List)
            return m_internalFileListEndpoint;
        else
            return m_internalDownloadEndpoint;
    }
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

