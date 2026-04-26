#include "downloader.hpp"
#include <QNetworkRequest>
#include <QDebug>

Downloader::Downloader(QObject* parent)
    : QObject(parent)
{
    mManager = new QNetworkAccessManager(this);
}

void Downloader::AddDownload(const AssetFile& assetFile, const QString& outputPath)
{
    mQueue.enqueue(Downloader::Job{assetFile, outputPath});
}

void Downloader::StartDownloads()
{
    mTotalDownloads = static_cast<int>(mQueue.size());
    mCurrentDownloadNum = 0;

    ProcessQueue();
}

void Downloader::ProcessQueue()
{
    if (!mQueue.isEmpty())
    {
        Job job = mQueue.dequeue();
        StartDownload(job);
        mCurrentDownloadNum++;
    }
    else
    {
        emit AllDownloadsFinished();
    }
}

void Downloader::StartDownload(const Job& job)
{
    QNetworkRequest request(job.assetFile.downloadUrl);

    request.setAttribute(QNetworkRequest::Attribute::RedirectPolicyAttribute, true);

    QNetworkReply* reply = mManager->get(request);

    QFile* file = new QFile(job.path);
    if (!file->open(QIODevice::WriteOnly))
    {
        emit failed(job.assetFile, "Failed to open file");
        delete file;
        return;
    }

    connect(reply, &QNetworkReply::readyRead, this, [reply, file]()
    {
        file->write(reply->readAll());
    });

    connect(reply, &QNetworkReply::downloadProgress, this, [this, job](qint64 received, qint64 total)
    {
        emit progress(job.assetFile, received, total);
    });

    connect(reply, &QNetworkReply::finished, this, [this, reply, file, job]()
    {
        file->write(reply->readAll());
        file->close();

        if (reply->error() == QNetworkReply::NoError)
        {
            emit finished(job.assetFile, job.path);
        }
        else
        {
            emit failed(job.assetFile, reply->errorString());
        }

        reply->deleteLater();
        file->deleteLater();

        ProcessQueue();
    });
}