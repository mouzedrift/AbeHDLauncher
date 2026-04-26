#pragma once

#include <QObject>
#include <QQueue>
#include <QUrl>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include "assetfile.hpp"

class Downloader : public QObject
{
    Q_OBJECT

public:
    explicit Downloader(QObject* parent = nullptr);

    void AddDownload(const AssetFile& assetFile, const QString& outputPath);
    void StartDownloads();
    int GetTotalDownloads() const { return mTotalDownloads; }
    int GetCurrentDownloadNum() const { return mCurrentDownloadNum; }

signals:
    void progress(const AssetFile& assetFile, qint64 received, qint64 total);
    void finished(const AssetFile& assetFile, const QString& path);
    void failed(const AssetFile& assetFile, const QString& error);
    void AllDownloadsFinished();

private:
    struct Job
    {
        AssetFile assetFile;
        QString path;
    };

    QQueue<Job> mQueue;
    QNetworkAccessManager* mManager;
    int mTotalDownloads = 0;
    int mCurrentDownloadNum = 0;

    void ProcessQueue();
    void StartDownload(const Job& job);
};
