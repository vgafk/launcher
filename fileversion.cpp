#include "fileversion.h"

FileData::FileData(int id, QString name, QObject *parent) : QObject(parent)
    ,m_id(id)
    ,m_name(name)
{}

bool FileData::operator==(const QString &otherName) const
{
    return m_name == otherName;
}


FileVersion::FileVersion(const QString &name, int versionMajor, int versionSubmajor, int versionMinor, int versionSubminor, QObject* parent) : QObject(parent)
{
    m_name = name;
    m_versionMajor = versionMajor;
    m_versionSubmajor = versionSubmajor;
    m_versionMinor = versionMinor;
    m_versionSubminor = versionSubminor;
}

bool FileVersion::operator==(const FileVersion &other) const{
    return  this->m_name == other.name() &&
           this->m_versionMajor == other.versionMajor() &&
           this->m_versionMajor == other.versionSubmajor() &&
           this->m_versionMajor == other.versionMinor() &&
           this->m_versionMajor == other.versionSubminor();
}

bool FileVersion::operator!=(const FileVersion &other) const{
    return !(*this == other);
}

void FileVersion::operator=(const FileVersion &other){
    this->m_name = other.name();
    this->m_versionMajor = other.versionMajor();
    this->m_versionMajor = other.versionSubmajor();
    this->m_versionMajor = other.versionMinor();
    this->m_versionMajor = other.versionSubminor();
}

bool FileVersion::operator<(const FileVersion *other)
{
    if (other->versionMajor() > versionMajor()) return true;
    if (other->versionMajor() < versionMajor()) return false;
    if (other->versionSubmajor() > versionSubmajor()) return true;
    if (other->versionSubmajor() < versionSubmajor()) return false;
    if (other->versionMinor() > versionMinor()) return true;
    if (other->versionMinor() < versionMinor()) return false;
    return other->versionSubminor() > versionSubminor();
}

int FileVersion::versionMajor() const
{
    return m_versionMajor;
}

int FileVersion::versionSubmajor() const
{
    return m_versionSubmajor;
}

int FileVersion::versionMinor() const
{
    return m_versionMinor;
}

int FileVersion::versionSubminor() const
{
    return m_versionSubminor;
}

QString FileVersion::name() const
{
    return m_name;
}

FileVersion *FileVersion::fromString(const QString &fileName, const QString &version, QObject *parent)
{
    QStringList versionNumbers = version.split(".");
    auto major = versionNumbers.at(0).toInt();
    auto smajor = versionNumbers.at(1).toInt();
    auto minor = versionNumbers.at(2).toInt();
    auto sminor = versionNumbers.count() > 3 ? versionNumbers.at(3).toInt() : 0;
    return new FileVersion(fileName, major, smajor, minor, sminor, parent);
}

QString FileVersion::toString() const
{
    return QString("%1.%2.%3.%4")
        .arg(m_versionMajor)
        .arg(m_versionSubmajor)
        .arg(m_versionMinor)
        .arg(m_versionSubminor);
}
