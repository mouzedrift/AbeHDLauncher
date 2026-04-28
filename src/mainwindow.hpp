#pragma once

#include <QMainWindow>
#include "assetfile.hpp"
#include "assetmanager.hpp"

namespace Ui
{
    class MainWindow;
}

enum class eGameType
{
    eAO,
    eAE
};

enum class ePlayButtonState
{
    eWaitForDownload,
    eWaitForLaunch,
};

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();

    static QString FindGameInstallPath(eGameType gameType);
private:

    void StartDownload(eGameType gameType);
    bool InstallRelive(eGameType gameType);
    void InstallSpritesAndCams(eGameType gameType);
    static QStringList ParseLibraryFoldersVdf(const QString& vdfPath);
    bool LaunchGame(eGameType gameType, bool bOnlyDoConversio);
    void SetPlayButtonState(ePlayButtonState state);
    void CloseGame();
    void OnSelectedGameChanged();
    void SetSelectedGame(eGameType game);
    bool IsGameInstalled(eGameType game, const QJsonDocument& infoJsonDocument);

    Ui::MainWindow* mUi = nullptr;
    class Downloader* mDownloader = nullptr;
    eGameType mSelectedGame;
    ePlayButtonState mPlayButtonState = ePlayButtonState::eWaitForLaunch;
    class QProcess* mGameProcess = nullptr;
    QList<AssetFile> mAssetsToInstall;
    bool mIsInstalling = false;
    AssetManager mAssetManager;
};
