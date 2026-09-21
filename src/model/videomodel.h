#ifndef VIDEOMODEL_H
#define VIDEOMODEL_H

#include <QDateTime>
#include <QString>
#include <QVector>

struct VideoSegment
{
    qint64 id = 0;
    int channel = 0;
    QString channelName;
    QDateTime startTime;
    QDateTime endTime;
    QString filePath;
    QString eventType;
};

class VideoModel
{
public:
    bool ensureTables(QString *error = nullptr) const;
    bool saveSegment(int channel,
                     const QString &channelName,
                     const QDateTime &startTime,
                     const QDateTime &endTime,
                     const QString &filePath,
                     const QString &eventType = "normal") const;
    QVector<VideoSegment> findSegments(const QDateTime &from,
                                       const QDateTime &to,
                                       int channel = 0,
                                       QString *error = nullptr) const;
    bool addSystemLog(const QString &level, const QString &message) const;
};

#endif
