#ifndef VIDEOMODEL_H
#define VIDEOMODEL_H

#include <QDateTime>
#include <QString>

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
    bool addSystemLog(const QString &level, const QString &message) const;
};

#endif
