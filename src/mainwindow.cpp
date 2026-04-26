#include "mainwindow.hpp"
#include "ui_mainwindow.h"
#include "downloader.hpp"
#include <vector>
#include <QSettings>
#include <QDir>
#include <QProcess>
#include "assetfile.hpp"
#include <QMessageBox>
#include <QPainter>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QJsonObject>
#include "assetsdialog.hpp"
#include <windows.h>
#include <dwmapi.h>

extern "C"
{
    #include <zip/zip.h>
}

static std::string CamNameToReliveDataPath(const std::string& camName, const std::string& gameInstallPath, eGameType gameType)
{
    // should be something like: MIP04C26.png
    if (camName.size() < 12)
    {
        return "";
    }

    std::string lvlAcronym(camName.substr(0, 2));

    int pathNum = std::stoi(camName.substr(3, 2));
    int camNum = std::stoi(camName.substr(6, 2));

    // remove leading zero if any
    std::string pathNumStr(std::to_string(pathNum));
    std::string camNumStr(std::to_string(camNum));

    std::transform(lvlAcronym.begin(), lvlAcronym.end(), lvlAcronym.begin(),[](unsigned char c) {return std::tolower(c);});

    std::string lvlFolderName;
    if (gameType == eGameType::eAE)
    {
        if (lvlAcronym == "mi")
        {
            lvlFolderName = "mines";
        }
        else if (lvlAcronym == "ne")
        {
            lvlFolderName = "necrum";
        }
        else if (lvlAcronym == "pv")
        {
            if (pathNum == 13)
            {
                lvlFolderName = "mudomo_vault_ender";
            }
            else
            {
                lvlFolderName = "mudomo_vault";
            }
        }
        else if (lvlAcronym == "sv")
        {
            // 14 = demo path
            if (pathNum == 9 || pathNum == 10 || pathNum == 11 || pathNum == 14)
            {
                lvlFolderName = "mudanchee_vault_ender";
            }
            else
            {
                lvlFolderName = "mudanchee_vault";
            }
        }
        else if (lvlAcronym == "fd")
        {
            if (pathNum == 11)
            {
                lvlFolderName = "feeco_depot_ender";
            }
            else
            {
                lvlFolderName = "feeco_depot";
            }
        }
        else if (lvlAcronym == "ba")
        {
            if (pathNum == 11 || pathNum == 16)
            {
                lvlFolderName = "barracks_ender";
            }
            else
            {
                lvlFolderName = "barracks";
            }
        }
        else if (lvlAcronym == "bw")
        {
            if (pathNum == 14 || pathNum == 9 || pathNum == 12)
            {
                lvlFolderName = "bonewerkz_ender";
            }
            else
            {
                lvlFolderName = "bonewerkz";
            }
        }
        else if (lvlAcronym == "br")
        {
            lvlFolderName = "brewery";
        }
        else if (lvlAcronym == "bm")
        {
            lvlFolderName = "brewery_ender";
        }
        else if (lvlAcronym == "cr")
        {
            lvlFolderName = "credits";
        }
        else if (lvlAcronym == "st")
        {
            lvlFolderName = "menu";
        }
        else
        {
            return "";
        }
    }
    else if (gameType == eGameType::eAO)
    {
        // TODO: ao lvl names
    }

    std::string gameStr = gameType == eGameType::eAE ? "ae" : "ao";
    return gameInstallPath + "/relive_data/" + gameStr + "/" + lvlFolderName + "/paths/" + pathNumStr + "/" + camNumStr + ".png";
}

static void EnableDarkTitleBar(HWND hwnd)
{
    BOOL value = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, &value, sizeof(value));
}

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    HWND hwnd = reinterpret_cast<HWND>(winId());
    EnableDarkTitleBar(hwnd);
    
    SetSelectedGame(eGameType::eAE);

    QDir().mkpath(GetAssetsPath());

    setFixedSize(size());

    mDownloader = new Downloader(this);

    connect(mDownloader, &Downloader::progress, this, [this](const AssetFile& assetFile, qint64 received, qint64 total)
    {
        if (total > 0 && received > 0)
        {
            double fileProgress = (static_cast<double>(received) / total);
            double overallProgress = ((mDownloader->GetCurrentDownloadNum() - 1) + fileProgress) / mDownloader->GetTotalDownloads();
            int progress = static_cast<int>(overallProgress * 100.0);

            mUi->downloadProgressBar->setValue(progress);

            double currentMB = received / (1000.0 * 1000.0);
            double totalMB = total / (1000.0 * 1000.0);
            mUi->downloadLabel->setText(QString("%1 (%2 / %3 MB) %4/%5").arg(assetFile.fileName).arg(currentMB, 0, 'f', 1).arg(totalMB, 0, 'f', 1).arg(mDownloader->GetCurrentDownloadNum()).arg(mDownloader->GetTotalDownloads()));
        }
    });

    connect(mDownloader, &Downloader::AllDownloadsFinished, this, [this]()
    {
        qDebug() << "all downloads finished";

        mUi->playButton->setEnabled(true);
        if (!InstallRelive(mSelectedGame))
        {
            QMessageBox::warning(this, "Warning", "Failed to install relive!");
            return;
        }

        // convert game data before installing sprites and cams so they dont get overridden
        LaunchGame(mSelectedGame, true);
    });

    connect(mDownloader, &Downloader::failed, this, [this](const AssetFile& assetFile, const QString& error)
    {
        qDebug() << "download of file" << assetFile.fileName << "failed with error" << error;
    });

    connect(mUi->playButton, &QPushButton::pressed, this, [this]()
    {
        if (mGameProcess && mPlayButtonState != ePlayButtonState::eWaitForLaunch)
        {
            CloseGame();

        }
        else if (!LaunchGame(mSelectedGame, false))
        {
            QMessageBox::warning(this, "Warning", "Couldn't launch game (make sure the game is installed on steam)");
        }
        else if (mPlayButtonState == ePlayButtonState::eWaitForExit)
        {
            SetPlayButtonState(ePlayButtonState::eWaitForLaunch);
        }
    });

    connect(mUi->discordButton, &QPushButton::pressed, this, []()
    {
        QDesktopServices::openUrl(QUrl("https://discord.com/invite/Ca5uB4JByA"));
    });

    connect(mUi->aoButton, &QPushButton::pressed, this, [this]()
    {
        SetSelectedGame(eGameType::eAO);
    });

    connect(mUi->aeButton, &QPushButton::pressed, this, [this]()
    {
        SetSelectedGame(eGameType::eAE);
    });

    connect(mUi->optionsButton, &QPushButton::pressed, this, [this]()
    {
        if (mSelectedGame == eGameType::eAO)
        {
            QMessageBox::warning(this, "Warning", "Oddysee is currently not supported");
            return;
        }

        auto pDialog = new AssetsDialog(this);
        connect(pDialog, &AssetsDialog::Accepted, this, [this](QList<AssetFile> assetsToInstall)
        {
            qDebug() << "selected " << assetsToInstall.size() << "assets for installation";
            mAssetsToInstall = assetsToInstall;
            mIsInstalling = true;
            mUi->optionsButton->setEnabled(false);

            StartDownload(mSelectedGame);
        });
        pDialog->exec();
        delete pDialog;
    });
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::StartDownload(eGameType gameType)
{
    for (const AssetFile& asset : mAssetsToInstall)
    {
        mDownloader->AddDownload(asset, GetAssetsPath() + "/" + asset.fileName);
    }

    mDownloader->StartDownloads();
}

bool MainWindow::InstallRelive(eGameType gameType)
{
    QString gameInstallPath = FindGameInstallPath(gameType);
    if (gameInstallPath.isEmpty())
    {
        return false;
    }

    QString zipAnimOutPath = gameInstallPath + "/relive_data/" + (gameType == eGameType::eAE ? "ae" : "ao") + "/animations";
    QDir().mkpath(zipAnimOutPath);

    QString zipFilePath = GetAssetsPath() + "/" + kReliveAsset.fileName;
    int error = zip_extract(zipFilePath.toUtf8().constData(), gameInstallPath.toUtf8().constData(), nullptr, nullptr);
    if (error != 0)
    {
        qDebug() << "Extraction failed for:" << zipFilePath << "with error " << zip_strerror(error);
        return false;
    }

    return true;
}

void MainWindow::InstallSpritesAndCams(eGameType gameType)
{
    QString gameInstallPath = FindGameInstallPath(gameType);
    if (gameInstallPath.isEmpty())
    {
        return;
    }

    QString zipAnimOutPath = gameInstallPath + "/relive_data/" + (gameType == eGameType::eAE ? "ae" : "ao") + "/animations";
    QDir().mkpath(zipAnimOutPath);

    for (const AssetFile& asset : mAssetsToInstall)
    {
        QString zipFilePath = GetAssetsPath() + "/" + asset.fileName;
        if (!QFile::exists(zipFilePath))
        {
            qDebug() << "skipping missing file" << zipFilePath;
            continue;
        }

        if (asset.fileId != eFileId::eCams) // extract sprites
        {
            qDebug() << "extracting" << asset.fileName << "to" << zipAnimOutPath;
            int error = zip_extract(zipFilePath.toUtf8().constData(), zipAnimOutPath.toUtf8().constData(), nullptr, nullptr);
            if (error != 0)
            {
                qDebug() << "Extraction failed for:" << zipFilePath << "with error" << zip_strerror(error);
            }
        }
        else if (asset.fileId == eFileId::eCams) // extract cams
        {
            zip_t* zip = zip_open(zipFilePath.toUtf8().constData(), 0, 'r');
            int n = zip_entries_total(zip);
            for (int i = 0; i < n; i++)
            {
                zip_entry_openbyindex(zip, i);

                const char* name = zip_entry_name(zip);
                if (!name)
                {
                    zip_entry_close(zip);
                    continue;
                }

                std::string filename(name);

                size_t pos = filename.find_last_of('/');
                if (pos != std::string::npos)
                {
                    filename = filename.substr(pos + 1);
                }

                std::string camOutPath = CamNameToReliveDataPath(filename, gameInstallPath.toUtf8().toStdString(), gameType);
                if (camOutPath.empty())
                {
                    zip_entry_close(zip);
                    qDebug() << "couldn't resolve camera output path for cam" << filename;
                    continue;
                }

                // read cam image into buffer
                size_t size = zip_entry_size(zip);
                std::vector<char> buffer(size);

                zip_entry_noallocread(zip, buffer.data(), size);

                zip_entry_close(zip);

                // write cam file
                QFile out(QString::fromStdString(camOutPath));
                if (out.open(QIODevice::WriteOnly))
                {
                    out.write(buffer.data(), static_cast<qint64>(size));
                    out.close();
                }

                qDebug() << "extracting" << asset.fileName << "to" << camOutPath;
            }

            zip_close(zip);
        }
    }

    QJsonObject obj;
    if (gameType == eGameType::eAO)
    {
        obj["OddyseeInstalled"] = true;
    }
    else
    {
        obj["ExoddusInstalled"] = true;
    }

    QJsonDocument doc(obj);

    QFile infoJson(GetInfoJsonPath());
    if (infoJson.open(QIODevice::WriteOnly))
    {
        infoJson.write(doc.toJson(QJsonDocument::Indented));
        infoJson.close();
    }

    SetPlayButtonState(ePlayButtonState::eWaitForLaunch);

    mUi->playButton->setEnabled(true);
    mUi->optionsButton->setEnabled(true);
    mAssetsToInstall.clear();
    mIsInstalling = false;

    QMessageBox::information(this, "Installation finished", "Installation finished");
}

QStringList MainWindow::ParseLibraryFoldersVdf(const QString& vdfPath)
{
    QStringList paths;

    QFile file(vdfPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return paths;
    }

    while (!file.atEnd())
    {
        QString line = file.readLine().trimmed();

        // extract library path
        if (line.startsWith("\"path\""))
        {
            qsizetype firstQuote = line.indexOf('"', 6);
            qsizetype secondQuote = line.indexOf('"', firstQuote + 1);

            QString path = line.mid(firstQuote + 1, secondQuote - firstQuote - 1);
            path.replace("\\\\", "/");

            paths.append(path);
        }
    }

    return paths;
}

QString MainWindow::FindGameInstallPath(eGameType gameType)
{
    QSettings settings("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
    QString steamPath = settings.value("SteamPath").toString();
    if (steamPath.isEmpty())
    {
        qDebug() << "SteamPath not found in registry";
        return "";
    }

    QString gameFolderName = gameType == eGameType::eAE ? "Oddworld Abes Exoddus" : "Oddworld Abes Oddysee";

    QString appPath = steamPath + "/steamapps/common/" + gameFolderName;
    if (QDir(appPath).exists())
    {
        return appPath;
    }

    // fallback
    QStringList libs = ParseLibraryFoldersVdf(steamPath + "/steamapps/libraryfolders.vdf");
    for (const QString& lib : libs)
    {
        qDebug() << "Found steam library:" << lib;
        appPath = lib + "/steamapps/common/" + gameFolderName;
        if (QDir(appPath).exists())
        {
            return appPath;
        }
    }

    return "";
}

bool MainWindow::LaunchGame(eGameType gameType, bool bOnlyDoConversion)
{
    QString gameInstallPath = FindGameInstallPath(gameType);
    if (gameInstallPath.isEmpty())
    {
        return false;
    }

    QString relivePath = gameInstallPath + "/relive.exe";
    if (!QFile::exists(relivePath))
    {
        return false;
    }

    if (mGameProcess)
    {
        CloseGame();
    }

    mGameProcess = new QProcess(this);
    mGameProcess->setWorkingDirectory(gameInstallPath);

    QStringList args;
    if (bOnlyDoConversion)
    {
        args.append("-only_convert_data");
    }

    mGameProcess->start(relivePath, args);

    connect(mGameProcess, &QProcess::finished,this, [this, bOnlyDoConversion, gameType](int exitCode, QProcess::ExitStatus status)
    {
        if (mGameProcess)
        {
            mGameProcess->deleteLater();
            mGameProcess = nullptr;

            if (bOnlyDoConversion && status == QProcess::ExitStatus::NormalExit)
            {
                InstallSpritesAndCams(gameType);
            }
            else
            {
                SetPlayButtonState(ePlayButtonState::eWaitForLaunch);
            }
        }
    });

    SetPlayButtonState(ePlayButtonState::eWaitForExit);

    return true;
}

void MainWindow::SetPlayButtonState(ePlayButtonState state)
{
    QString gameName = mSelectedGame == eGameType::eAO ? "ODDYSEE" : "EXODDUS";
    if (state == ePlayButtonState::eWaitForLaunch || state == ePlayButtonState::eWaitForDownload)
    {
        mUi->playButton->setText("PLAY " + gameName + " HD");
    }
    else if (state == ePlayButtonState::eWaitForExit)
    {
        mUi->playButton->setText("CLOSE GAME");
    }

    mPlayButtonState = state;
}

void MainWindow::CloseGame()
{
    if (mGameProcess)
    {
        mGameProcess->kill();
        mGameProcess->deleteLater();
        mGameProcess = nullptr;
    }
}

void MainWindow::SetSelectedGame(eGameType game)
{
    if (mIsInstalling)
    {
        return;
    }

    mSelectedGame = game;

    bool bIsGameInstalled = IsGameInstalled(game);
    mUi->playButton->setDisabled(!bIsGameInstalled);

    if (!bIsGameInstalled)
    {
        SetPlayButtonState(ePlayButtonState::eWaitForDownload);
    }
    else
    {
        SetPlayButtonState(ePlayButtonState::eWaitForLaunch);
    }

    // force repaint on game change for the background image
    update();
}

bool MainWindow::IsGameInstalled(eGameType game)
{
    if (!QFile::exists(GetInfoJsonPath()))
    {
        return false;
    }

    QFile infoJson(GetInfoJsonPath());
    infoJson.open(QIODevice::ReadOnly);

    QJsonDocument doc = QJsonDocument::fromJson(infoJson.readAll());
    QJsonObject obj = doc.object();

    if (game == eGameType::eAO)
    {
        return obj["OddyseeInstalled"].toBool();
    }

    return obj["ExoddusInstalled"].toBool();
}
