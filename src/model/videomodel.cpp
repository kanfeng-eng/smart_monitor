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

bool VideoModel::addSystemLog(const QString &level, const QString &message) const
{
    QSqlQuery query(DbConn::getInstance().database());
    query.prepare("INSERT INTO system_logs (level, message) VALUES (:level, :message)");
    query.bindValue(":level", level);
    query.bindValue(":message", message);
    return query.exec();
}
