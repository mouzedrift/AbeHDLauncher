#include "assetsdialog.hpp"
#include "ui_assetsdialog.h"
#include <QButtonGroup>

static bool IsDownloaded(const AssetFile& assetFile)
{
    return QFile::exists(GetAssetsPath() + "/" + assetFile.fileName);
}

AssetsDialog::AssetsDialog(QWidget* pParent)
    : QDialog(pParent)
    , mUi(new Ui::AssetsDialog)
{
    mUi->setupUi(this);

    connect(mUi->ConfirmButton, &QPushButton::pressed, this, [this]()
    {
        QList<AssetFile> assetsToInstall;
        if (!IsDownloaded(kReliveAsset))
        {
            assetsToInstall.append(kReliveAsset);
        }

        for (auto& asset : kSpritesAndCamsAssets)
        {
            if (IsDownloaded(asset))
            {
                continue;
            }

            if (asset.fileId == eFileId::eAbeOriginal)
            {
                if (mUi->abeOriginalRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eAbeFMV)
            {
                if (mUi->abeFMVRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }

            if (asset.fileId == eFileId::eSligAENoTubes)
            {
                if (mUi->sligAENoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eSligAETubes)
            {
                if (mUi->sligAETubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eSligAONoTubes)
            {
                if (mUi->sligAONoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eSligAOTubes)
            {
                if (mUi->sligAOTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eSligMONoTubes)
            {
                if (mUi->sligMONoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == eFileId::eSligMOTubes)
            {
                if (mUi->sligMOTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }

            assetsToInstall.append(asset);
        }

        emit Accepted(assetsToInstall);
        close();
    });
}

AssetsDialog::~AssetsDialog()
{
    delete mUi;
}
