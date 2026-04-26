#pragma once

#include <QDialog>
#include "assetfile.hpp"
#include <QList>

namespace Ui
{
    class AssetsDialog;
}

class AssetsDialog final : public QDialog
{
    Q_OBJECT

public:
    explicit AssetsDialog(QWidget* pParent = nullptr);
    ~AssetsDialog();

signals:
    void Accepted(QList<AssetFile> assetsToInstall);

private:
    Ui::AssetsDialog* mUi = nullptr;
    AssetFile mSelectedAbeSkin;
    AssetFile mSelectedSligSkin;
};
