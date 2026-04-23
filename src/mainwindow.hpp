#pragma once

#include <QMainWindow>
#include "assetfile.hpp"

namespace Ui
{
    class MainWindow;
}

enum class eGameType
{
    eAO,
    eAE
};

enum class eBigButtonState
{
    eWaitForDownload,
    eWaitForLaunchGame,
    eGameLaunched
};

class MainWindow final : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* pParent = nullptr);
    ~MainWindow();

    static QString FindGameInstallPath(eGameType gameType);
private:

    void paintEvent(class QPaintEvent*) override;
    void StartDownload(eGameType gameType);
    bool InstallRelive(eGameType gameType);
    void InstallSpritesAndCams(eGameType gameType);
    static QStringList ParseLibraryFoldersVdf(const QString& vdfPath);
    bool LaunchGame(eGameType gameType, bool bOnlyDoConversio);
    void SetBigButtonState(eBigButtonState state);
    void CloseGame();
    void OnSelectedGameChanged();
    void SetSelectedGame(eGameType game);
    bool IsGameInstalled(eGameType game);

    Ui::MainWindow* mUi = nullptr;
    class Downloader* mDownloader = nullptr;
    eGameType mSelectedGame;
    QPixmap mAEBackground;
    QPixmap mAOBackground;
    eBigButtonState mBigButtonState = eBigButtonState::eWaitForDownload;
    class QProcess* mGameProcess = nullptr;
    QList<AssetFile> mAssetsToInstall;
};
