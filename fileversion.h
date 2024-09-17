#ifndef FILEVERSION_H
#define FILEVERSION_H

#include <QObject>

class FileVersion : public QObject
{
    Q_OBJECT

public:
    explicit FileVersion(int versionMajor, int versionSubmajor, int versionMinor, int versionSubminor, QObject *parent);
    ~FileVersion(){};

    bool operator==(const FileVersion &other) const;
    bool operator!=(const FileVersion &other) const;
    void operator=(const FileVersion &other);

    int versionMajor() const;
    int versionSubmajor() const;
    int versionMinor() const;
    int versionSubminor() const;

    static FileVersion* fromString(const QString &version, QObject *parent);
    QString toString() const;

private:
    int m_versionMajor;
    int m_versionSubmajor;
    int m_versionMinor;
    int m_versionSubminor;
};

#endif // FILEVERSION_H
