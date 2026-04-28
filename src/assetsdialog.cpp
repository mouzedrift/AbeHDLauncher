#include "assetsdialog.hpp"
#include "ui_assetsdialog.h"
#include <QButtonGroup>
#include "assetmanager.hpp"

AssetsDialog::AssetsDialog(AssetManager& assetManager, QWidget* pParent)
    : QDialog(pParent)
    , mUi(new Ui::AssetsDialog)
    , mAssetManager(assetManager)
{
    mUi->setupUi(this);

    connect(mUi->ConfirmButton, &QPushButton::pressed, this, [this]()
    {
        QList<AssetFile> assetsToInstall;
        for (const auto& asset : mAssetManager.Assets())
        {
            if (asset.fileId == "abe_original")
            {
                if (mUi->abeOriginalRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "abe_fmv")
            {
                if (mUi->abeFMVRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }

            if (asset.fileId == "slig_ae_no_tubes")
            {
                if (mUi->sligAENoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "slig_ae_tubes")
            {
                if (mUi->sligAETubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "slig_ao_no_tubes")
            {
                if (mUi->sligAONoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "slig_ao_tubes")
            {
                if (mUi->sligAOTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "slig_mo_no_tubes")
            {
                if (mUi->sligMONoTubesRadioButton->isChecked())
                {
                    assetsToInstall.append(asset);
                }
                continue;
            }
            else if (asset.fileId == "slig_mo_tubes")
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

    mUi->SligAENoTubes->hide();
    mUi->SligAETubes->hide();
    mUi->SligAONoTubes->hide();
    mUi->SligAOTubes->hide();
    mUi->SligMONoTubes->hide();
    mUi->SligMOTubes->hide();

    // default skin for AE
    mUi->SligAONoTubes->show();
    mUi->sligAONoTubesRadioButton->setChecked(true);
}

AssetsDialog::~AssetsDialog()
{
    delete mUi;
}
