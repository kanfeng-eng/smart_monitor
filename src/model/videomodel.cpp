#include "videomodel.h"
#include "dbconn.h"

#include <QSqlError>
#include <QSqlQuery>
#include <QStringList>
#include <QVariant>

bool VideoModel::ensureTables(QString *error) const
{
    QSqlQuery query(DbConn::getInstance().database());
    const QStringList statements = {
        "CREATE TABLE IF NOT EXISTS video_segments ("
        "segment_id BIGINT PRIMARY KEY AUTO_INCREMENT,"
        "channel_no INT NOT NULL,"
        "channel_name VARCHAR(64) NOT NULL,"
        "start_time DATETIME NOT NULL,"
        "end_time DATETIME NOT NULL,"
        "file_path VARCHAR(512) NOT NULL,"
        "event_type VARCHAR(32) NOT NULL DEFAULT 'normal',"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "INDEX idx_video_query (start_time, channel_no, event_type))",
        "CREATE TABLE IF NOT EXISTS system_logs ("
        "log_id BIGINT PRIMARY KEY AUTO_INCREMENT,"
        "level VARCHAR(16) NOT NULL,"
        "message VARCHAR(512) NOT NULL,"
        "created_at DATETIME DEFAULT CURRENT_TIMESTAMP,"
        "INDEX idx_log_time (created_at))"
    };

    for (const QString &statement : statements)
    {
        if (!query.exec(statement))
        {
            if (error)
                *error = query.lastError().text();
            return false;
        }
    }
    return true;
}

bool VideoModel::saveSegment(int channel,
                             const QString &channelName,
                             const QDateTime &startTime,
                             const QDateTime &endTime,
                             const QString &filePath,
                             const QString &eventType) const
{
    QSqlQuery query(DbConn::getInstance().database());
    query.prepare("INSERT INTO video_segments "
                  "(channel_no, channel_name, start_time, end_time, file_path, event_type) "
                  "VALUES (:channel, :name, :start, :end, :path, :type)");
    query.bindValue(":channel", channel);
    query.bindValue(":name", channelName);
    query.bindValue(":start", startTime);
    query.bindValue(":end", endTime);
    query.bindValue(":path", filePath);
    query.bindValue(":type", eventType);
    return query.exec();
}

QVector<VideoSegment> VideoModel::findSegments(const QDateTime &from,
                                               const QDateTime &to,
                                               int channel,
                                               QString *error) const
{
    QVector<VideoSegment> segments;
    QSqlDatabase database = DbConn::getInstance().database();
    if (!database.isOpen())
    {
        if (error)
            *error = "数据库未连接";
        return segments;
    }

    QSqlQuery query(database);
    QString sql = "SELECT segment_id, channel_no, channel_name, start_time, end_time, "
                  "file_path, event_type FROM video_segments "
                  "WHERE start_time < :to_time AND end_time >= :from_time";
    if (channel > 0)
        sql += " AND channel_no = :channel";
    sql += " ORDER BY start_time DESC";
    query.prepare(sql);
    query.bindValue(":from_time", from);
    query.bindValue(":to_time", to);
    if (channel > 0)
        query.bindValue(":channel", channel);

    if (!query.exec())
    {
        if (error)
            *error = query.lastError().text();
        return segments;
    }

    while (query.next())
    {
        VideoSegment segment;
        segment.id = query.value(0).toLongLong();
        segment.channel = query.value(1).toInt();
        segment.channelName = query.value(2).toString();
        segment.startTime = query.value(3).toDateTime();
        segment.endTime = query.value(4).toDateTime();
        segment.filePath = query.value(5).toString();
        segment.eventType = query.value(6).toString();
        segments.append(segment);
    }
    return segments;
}

bool VideoModel::addSystemLog(const QString &level, const QString &message) const
{
    QSqlQuery query(DbConn::getInstance().database());
    query.prepare("INSERT INTO system_logs (level, message) VALUES (:level, :message)");
    query.bindValue(":level", level);
    query.bindValue(":message", message);
    return query.exec();
}
