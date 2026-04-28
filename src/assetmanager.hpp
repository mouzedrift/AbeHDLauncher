#pragma once

#include "assetfile.hpp"
#include <vector>

class AssetManager
{
public:
    QString GetAssetsPath() const;
    bool Load();

    const std::vector<AssetFile>& Assets() const { return mAssets; }
    const AssetFile* FindById(const QString& id) const;
    QString GetAssetFilePath(const AssetFile& assetFile) const;
    bool IsDownloaded(const AssetFile& assetFile) const;

private:
    std::vector<AssetFile> mAssets;
};