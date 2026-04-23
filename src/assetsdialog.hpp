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

    void Populate(const std::vector<AssetFile>& assets);

signals:
    void Accepted(QList<AssetFile> assetFiles);

private:
    Ui::AssetsDialog* mUi = nullptr;
};
