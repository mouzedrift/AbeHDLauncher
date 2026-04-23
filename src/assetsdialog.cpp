#include "assetsdialog.hpp"
#include "ui_assetsdialog.h"

AssetsDialog::AssetsDialog(QWidget* pParent)
    : QDialog(pParent)
    , mUi(new Ui::AssetsDialog)
{
    mUi->setupUi(this);

    connect(mUi->selectAllButton, &QPushButton::pressed, this, [this]()
    {
        for (int i = 0; i < mUi->listWidget->count(); i++)
        {
            QListWidgetItem* pItem = mUi->listWidget->item(i);
            pItem->setCheckState(Qt::Checked);
         }
    });

    connect(mUi->unselectAllButton, &QPushButton::pressed, this, [this]()
    {
        for (int i = 0; i < mUi->listWidget->count(); i++)
        {
            QListWidgetItem* pItem = mUi->listWidget->item(i);
            pItem->setCheckState(Qt::Unchecked);
        }
    });

    connect(mUi->buttonBox, &QDialogButtonBox::accepted, this, [this]()
    {
        QList<AssetFile> assetFiles;
        for (int i = 0; i < mUi->listWidget->count(); i++)
        {
            QListWidgetItem* pItem = mUi->listWidget->item(i);
            if (pItem->checkState() == Qt::Checked)
            {
                assetFiles.append(pItem->data(Qt::UserRole).value<AssetFile>());
            }
        }

        emit Accepted(assetFiles);
    });
}

AssetsDialog::~AssetsDialog()
{
    delete mUi;
}

void AssetsDialog::Populate(const std::vector<AssetFile>& assets)
{
    for (auto& asset : assets)
    {
        QListWidgetItem* pItem = new QListWidgetItem(mUi->listWidget);
        pItem->setText(asset.fileName);
        pItem->setData(Qt::UserRole, QVariant::fromValue(asset));
        pItem->setFlags(pItem->flags() | Qt::ItemIsUserCheckable);
        pItem->setCheckState(Qt::Unchecked);
    }
}
