#include "assetmanager.hpp"
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QJsonArray>
#include <QDir>

QString AssetManager::GetAssetsPath() const
{
    return QCoreApplication::applicationDirPath() + "/assets";
}

bool AssetManager::Load()
{
    QDir().mkpath(GetAssetsPath());

    QString assetsJsonPath = QCoreApplication::applicationDirPath() + "/assets.json";
    QFile assetsFile(assetsJsonPath);
    if (!assetsFile.open(QIODevice::ReadOnly))
    {
        QMessageBox::critical(nullptr, "Error", "Couldn't open assets.json for reading");
        return false;
    }

    QJsonDocument jsonDocument;
    QJsonParseError error;
    jsonDocument = QJsonDocument::fromJson(assetsFile.readAll(), &error);
    if (error.error != QJsonParseError::NoError)
    {
        QMessageBox::critical(nullptr, "Error", "Error parsing assets.json");
        return false;
    }

    QJsonObject root = jsonDocument.object();
    if (!root.contains("files") || !root["files"].isArray())
    {
        QMessageBox::critical(nullptr, "Error", "Invalid assets.json format (missing files array)");
        return false;
    }

    QJsonArray filesArray = root["files"].toArray();
    if (filesArray.isEmpty())
    {
        QMessageBox::critical(nullptr, "Error", "assets.json is empty");
        return false;
    }

    mAssets.clear();
    mAssets.reserve(filesArray.count());

    for (const auto& val : filesArray)
    {
        QJsonObject assetObj = val.toObject();

        AssetFile assetFile;
        assetFile.fileId = assetObj["id"].toString();
        assetFile.downloadUrl = assetObj["url"].toVariant().toUrl();
        assetFile.fileName = assetObj["name"].toString();

        mAssets.push_back(assetFile);
    }

    return true;
}

const AssetFile* AssetManager::FindById(const QString& id) const
{
    for (const auto& asset : mAssets)
    {
        if (asset.fileId == id)
        {
            return &asset;
        }
    }

    return nullptr;
}

QString AssetManager::GetAssetFilePath(const AssetFile& assetFile) const
{
    return QDir(GetAssetsPath()).filePath(assetFile.fileName);
}

bool AssetManager::IsDownloaded(const AssetFile& assetFile) const
{
    return QFile::exists(GetAssetFilePath(assetFile));
}