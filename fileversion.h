#ifndef FILEVERSION_H
#define FILEVERSION_H

#include <QObject>

class FileData : public QObject
{
    explicit FileData(int id, QString name, QObject *parent);

    bool operator==(const QString &otherName) const;

private:
    const int m_id;
    const QString m_name;
};

class FileVersion : public QObject
{
    Q_OBJECT

public:
    explicit FileVersion(const QString &name, int versionMajor, int versionSubmajor, int versionMinor, int versionSubminor, QObject *parent);
    ~FileVersion(){}

    bool operator==(const FileVersion &other) const;
    bool operator!=(const FileVersion &other) const;
    void operator=(const FileVersion &other);

    bool operator<(const FileVersion *other);

    int versionMajor() const;
    int versionSubmajor() const;
    int versionMinor() const;
    int versionSubminor() const;
    QString name() const;

    static FileVersion* fromString(const QString &fileName, const QString &version, QObject *parent);
    QString toString() const;

private:
    QString m_name;
    int m_versionMajor;
    int m_versionSubmajor;
    int m_versionMinor;
    int m_versionSubminor;
};

#endif // FILEVERSION_H
