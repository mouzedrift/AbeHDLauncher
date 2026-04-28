#pragma once

#include <QUrl>
#include <QFile>
#include <QCoreApplication>

struct AssetFile
{
    QUrl downloadUrl;
    QString fileName;
    QString fileId;
};
