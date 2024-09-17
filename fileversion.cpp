#include "fileversion.h"

FileVersion::FileVersion(int versionMajor, int versionSubmajor, int versionMinor, int versionSubminor, QObject* parent) : QObject(parent)
{
    m_versionMajor = versionMajor;
    m_versionSubmajor = versionSubmajor;
    m_versionMinor = versionMinor;
    m_versionSubminor = versionSubminor;
}

bool FileVersion::operator==(const FileVersion &other) const{
    return this->m_versionMajor == other.versionMajor() &&
            this->m_versionMajor == other.versionSubmajor() &&
            this->m_versionMajor == other.versionMinor() &&
            this->m_versionMajor == other.versionSubminor();
}

bool FileVersion::operator!=(const FileVersion &other) const{
    return !(*this == other);
}

void FileVersion::operator=(const FileVersion &other){
    this->m_versionMajor = other.versionMajor();
    this->m_versionMajor = other.versionSubmajor();
    this->m_versionMajor = other.versionMinor();
    this->m_versionMajor = other.versionSubminor();
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

FileVersion *FileVersion::fromString(const QString &version, QObject *parent)
{
    QStringList versionNumbers = version.split(".");
    auto major = versionNumbers.at(0).toInt();
    auto smajor = versionNumbers.at(1).toInt();
    auto minor = versionNumbers.at(2).toInt();
    auto sminor = versionNumbers.count() > 3 ? versionNumbers.at(3).toInt() : 0;
    return new FileVersion(major, smajor, minor, sminor, parent);
}

QString FileVersion::toString() const
{
    return QString("%1.%2.%3.%4")
            .arg(m_versionMajor)
            .arg(m_versionSubmajor)
            .arg(m_versionMinor)
            .arg(m_versionSubminor);
}
