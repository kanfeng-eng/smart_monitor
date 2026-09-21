#include "../src/view/cameraworker.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QTimer>
#include <QVector>

#include <cstdio>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    const QString outputRoot = QDir::temp().filePath("smart_monitor_camera_test");
    QDir(outputRoot).removeRecursively();
    QDir().mkpath(outputRoot);

    QVector<CameraWorker *> workers;
    int savedSegments = 0;
    bool recordingFailed = false;
    for (int i = 0; i < 4; ++i)
    {
        CameraWorker *worker = new CameraWorker(99 + i,
                                                QString("测试通道 %1").arg(i + 1),
                                                outputRoot);
        workers.append(worker);
        QObject::connect(worker, &CameraWorker::segmentFinished,
                         [&](int, const QString &, const QDateTime &, const QDateTime &,
                             const QString &filePath, const QString &) {
            const QFileInfo file(filePath);
            if (!file.exists() || file.size() <= 0)
            {
                recordingFailed = true;
                return;
            }
            std::fprintf(stderr, "saved channel file: %lld bytes\n",
                         static_cast<long long>(file.size()));
            ++savedSegments;
            if (savedSegments == 4)
                QCoreApplication::quit();
        });
        QObject::connect(worker, &CameraWorker::recordingError,
                         [&](const QString &) { recordingFailed = true; });
        worker->start();
        worker->setRecording(true);
    }
    QTimer::singleShot(1500, [&]() {
        for (CameraWorker *worker : workers)
            worker->stop();
    });
    QTimer::singleShot(5000, &app, &QCoreApplication::quit);
    app.exec();

    const bool passed = savedSegments == 4 && !recordingFailed;
    qDeleteAll(workers);
    QDir(outputRoot).removeRecursively();
    return passed ? 0 : 1;
}
