CREATE DATABASE IF NOT EXISTS smart_monitor
DEFAULT CHARACTER SET utf8mb4
COLLATE utf8mb4_unicode_ci;

USE smart_monitor;


CREATE TABLE admins (
    admin_id INT PRIMARY KEY AUTO_INCREMENT,

    username VARCHAR(10) NOT NULL UNIQUE,

    password VARCHAR(32) NOT NULL,

    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS video_segments (
    segment_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    channel_no INT NOT NULL,
    channel_name VARCHAR(64) NOT NULL,
    start_time DATETIME NOT NULL,
    end_time DATETIME NOT NULL,
    file_path VARCHAR(512) NOT NULL,
    event_type VARCHAR(32) NOT NULL DEFAULT 'normal',
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_video_query (start_time, channel_no, event_type)
);

CREATE TABLE IF NOT EXISTS system_logs (
    log_id BIGINT PRIMARY KEY AUTO_INCREMENT,
    level VARCHAR(16) NOT NULL,
    message VARCHAR(512) NOT NULL,
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
    INDEX idx_log_time (created_at)
);
