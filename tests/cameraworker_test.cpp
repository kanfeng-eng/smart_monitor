#include "../src/view/cameraworker.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>

#include <cassert>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QString outputRoot = QDir::temp().filePath("smart_monitor_camera_test");
    QDir(outputRoot).removeRecursively();
    QDir().mkpath(outputRoot);

    CameraWorker worker(99, "测试通道", outputRoot);
    bool segmentSaved = false;
    QObject::connect(&worker, &CameraWorker::segmentFinished,
                     [&](int, const QString &, const QDateTime &, const QDateTime &,
                         const QString &filePath, const QString &) {
        const QFileInfo file(filePath);
        segmentSaved = file.exists() && file.size() > 0;
        QCoreApplication::quit();
    });
    QObject::connect(&worker, &CameraWorker::recordingError,
                     [](const QString &) { assert(false); });

    worker.start();
    worker.setRecording(true);
    QTimer::singleShot(1500, &worker, &CameraWorker::stop);
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);
    app.exec();

    assert(segmentSaved);
    QDir(outputRoot).removeRecursively();
    return 0;
}
