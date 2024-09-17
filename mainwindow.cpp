#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    setBase();
    checkUpdate();
    runExecutable();
}

MainWindow::~MainWindow()
{
    delete ui;
}


bool MainWindow::setBase()
{
    m_base = QSqlDatabase::addDatabase("QMYSQL");
    m_base.setDatabaseName("corusant");
    m_base.setUserName("ordo");
    m_base.setPassword("ordo7532159");
    if(!connectToBase(serverAddress(Local))){
        if(!connectToBase(serverAddress(Remote))){
            ui->label->setText("Не могу подключится к серверу, проверте подключение к сети");
            return false;
        }
    }
    saveServer(m_base.hostName(), m_base.port());
    return true;
}

bool MainWindow::connectToBase(QPair<QString, int> server)
{
    m_base.setHostName(server.first);
    m_base.setPort(server.second);
    return m_base.open();
}

QPair<QString, int> MainWindow::serverAddress(Server server)
{
    QSettings s("settings.ini", QSettings::IniFormat);
    switch (server) {
    case Remote:
        return qMakePair(s.value("remoteHost", "83.167.69.146").toString(), s.value("remotePort", 5806).toInt());
    default:
        return qMakePair(s.value("localHost", "10.0.2.18").toString(), s.value("localPort", 3306).toInt());
    }
}

void MainWindow::saveServer(QString host, int port)
{
     QSettings s("settings.ini", QSettings::IniFormat);
     s.setValue("dbHost", host);
     s.setValue("dbPort", port);
}

QMap<QString, FileVersion*> MainWindow::baseFileList()
{
    QSqlQuery query;
    QMap<QString, FileVersion*> files;
    query.exec("SELECT file_name, version_major, version_submajor, version_minor, version_subminor "
               "FROM files_update ");

    if(m_base.lastError().isValid()){
        ui->label->setText(QString("%1\n\n%2").arg(ui->label->text()).arg(m_base.lastError().text()));
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
    for(auto file: files){
        QString version = settings.value(file).toString();
        fileVersions.insert(file, FileVersion::fromString(version, this));
    }
    return fileVersions;
}

bool MainWindow::checkUpdate()
{
    auto baseFiles = baseFileList();
    auto localFiles = localFileList();

    for(auto bf : baseFiles.keys()){
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
    QSqlQuery query;
    query.exec(QString("SELECT file FROM files_update WHERE file_name = '%1'").arg(f.fileName()));

    if(query.lastError().isValid()){
        ui->label->setText(QString("%1\n\n%2").arg(ui->label->text()).arg(query.lastError().text()));
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

void MainWindow::runExecutable()
{
    QSettings settings("settings.ini", QSettings::IniFormat);
    auto file = settings.value("executable", m_defExecutable).toString();
    QProcess::startDetached(file, QStringList());
}

