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

extern "C"
{
    #include <zip/zip.h>
}

static const std::vector<AssetFile> sAssetFiles =
{
    { QUrl("https://www.dropbox.com/scl/fi/ru5tc9425fjikuisiubs8/relive.zip?rlkey=xdf0b9hmmb10312vqnqh4y1ug&st=mik2tfhf&dl=1"), "relive.zip", eFileId::eRelive},
    { QUrl("https://www.dropbox.com/scl/fi/cr1s9e4y7srh2asx49wbu/dove.zip?rlkey=111h7t1kr6ej64ae3mvak6pp7&st=bv148mb7&dl=1"), "dove.zip", eFileId::eDove},
    { QUrl("https://www.dropbox.com/scl/fi/uu1lv3qk3y2jhnle5h96w/abe-FMV.zip?rlkey=34tgxoly8ysccp950ublj9c96&st=p5fwai3r&dl=1"), "abe-FMV.zip" , eFileId::eAbeFMV},
    { QUrl("https://www.dropbox.com/scl/fi/5b740nok9gndhoz26gsui/abe-original.zip?rlkey=16eloohbop2m5fqdm6w6e9832&st=9r9jk6kj&dl=1"), "abe-original.zip" , eFileId::eAbeOriginal},
    { QUrl("https://www.dropbox.com/scl/fi/kajb1r41mj946m5wx6v56/boom_machine.zip?rlkey=ifkteann6ml78wjgqvqm5ovp3&st=p6hla4kc&dl=1"), "boom_machine.zip" , eFileId::eBoomMachine},
    { QUrl("https://www.dropbox.com/scl/fi/o12575b36vv8nxhnxov2u/crawling_slig.zip?rlkey=3tj4huytjzycctx2j7wzgf380&st=wml9xagg&dl=1"), "crawling_slig.zip" , eFileId::eCrawlingSlig},
    { QUrl("https://www.dropbox.com/scl/fi/75ain0sdqpnybj1ssgqqn/door_lock.zip?rlkey=b3aup001wl1qjam8auwao4ofk&st=9fo3s03p&dl=1"), "door_lock.zip" , eFileId::eDoorLock},
    { QUrl("https://www.dropbox.com/scl/fi/7ivho12tuenf83li0e0hr/doors.zip?rlkey=mjt5wh8qw66yp79s1nowgf4el&st=nmv68ulu&dl=1"), "doors.zip" , eFileId::eDoors},
    { QUrl("https://www.dropbox.com/scl/fi/82gcz0fwmlfeuv5crod08/drill.zip?rlkey=e9gzmtv8ac5ta5jcc3vnqxdpg&st=179nsmkl&dl=1"), "drill.zip", eFileId::eDrill },
    { QUrl("https://www.dropbox.com/scl/fi/g3wivo2kldrds2jivxv53/electric_wall.zip?rlkey=iznj8e339ji5r5jqh830sgdzi&st=jil1o8hd&dl=1"), "electric_wall.zip", eFileId::eElectricWall },
    { QUrl("https://www.dropbox.com/scl/fi/507pwwwf68xkwzy4s6q9w/falling_items.zip?rlkey=6py0kz1vwotgjz8tm7c8ap41i&st=isw8corv&dl=1"), "falling_items.zip", eFileId::eFallingItems },
    { QUrl("https://www.dropbox.com/scl/fi/ilx36089qd6va8uhcgsut/fleech.zip?rlkey=gxuvnb3hrkc7xkmqqhk8pjas4&st=6q7ywe6x&dl=1"), "fleech.zip", eFileId::eFleech },
    { QUrl("https://www.dropbox.com/scl/fi/t421sknpmkx6u1uuayf00/flying_slig.zip?rlkey=vxlvovo8n2ycf5hew8654i1r1&st=kr7s18t2&dl=1"), "flying_slig.zip", eFileId::eFlyingSlig},
    { QUrl("https://www.dropbox.com/scl/fi/tdlxmp89k1o21a5nz5999/foot_switches.zip?rlkey=l11am04lbgchb4zdbnf5qnrhl&st=1ulww43n&dl=1"), "foot_switches.zip", eFileId::eFootSwitches },
    { QUrl("https://www.dropbox.com/scl/fi/2avgnl94d3nwttedpx35h/gibs.zip?rlkey=6pzawyhjze9ipppvqs6m9j9ul&st=rjtyucaf&dl=1"), "gibs.zip", eFileId::eGibs },
    { QUrl("https://www.dropbox.com/scl/fi/lwayhtifafxjy76prvf4v/glukkons.zip?rlkey=p5bwlgch6usl2tb73ueut6yhy&st=lzl78k4s&dl=1"), "glukkons.zip", eFileId::eGlukkons },
    { QUrl("https://www.dropbox.com/scl/fi/mw6qerway3xpyvukhbfas/greeter.zip?rlkey=9kk9exn9rhqjtauiiai4bf4wm&st=v65ste5w&dl=1"), "greeter.zip", eFileId::eGreeter },
    { QUrl("https://www.dropbox.com/scl/fi/65vcyfj2uxegog4g3d2k8/icons.zip?rlkey=vzzgkgzt1mgu1hgwtqmu06x1r&st=5teekxby&dl=1"), "icons.zip", eFileId::eIcons },
    { QUrl("https://www.dropbox.com/scl/fi/tn67qo0ca1vxgp1x5q3jw/menu_glukkon_speak.zip?rlkey=fxmnard5ln7wzlbl10767zq4k&st=lna9sfiw&dl=1"), "menu_glukkon_speak.zip", eFileId::eMenuGlukkonSpeak },
    { QUrl("https://www.dropbox.com/scl/fi/n5167bea7ifc1ohojnqeg/mine_car.zip?rlkey=8ncsvdrc5jrqxzuxqe7lpuk0t&st=1p7uc6co&dl=1"), "mine_car.zip", eFileId::eMineCar },
    { QUrl("https://www.dropbox.com/scl/fi/w4oajzkdc904xbo7afj4y/motion_detector.zip?rlkey=86ejhql7pt4h8iuibr913boek&st=km7bixte&dl=1"), "motion_detector.zip", eFileId::eMotionDetector },
    { QUrl("https://www.dropbox.com/scl/fi/7a0rv3xsjz06oj2gks6yq/paramite.zip?rlkey=ltqevr280pg25v4i2ud9gk012&st=d624pnjk&dl=1"), "paramite.zip", eFileId::eParamite },
    { QUrl("https://www.dropbox.com/scl/fi/qqgcvpe4ydswz3dqez4fw/pull_ring_rope.zip?rlkey=8xvt05e8tpmwtfeoqks5cu295&st=d3a1adxg&dl=1"), "pull_ring_rope.zip", eFileId::ePullRingRope },
    { QUrl("https://www.dropbox.com/scl/fi/w03s3xfecya19sjoy9ed8/rock_sack.zip?rlkey=34o9o4rr7i0yjye10mejctsxq&st=hs5zzszd&dl=1"), "rock_sack.zip", eFileId::eRockSack },
    { QUrl("https://www.dropbox.com/scl/fi/zve2171j2sfes4vzd183c/ropes.zip?rlkey=7f2iuobisc35r8ig2hm9jjze6&st=ew84kes1&dl=1"), "ropes.zip", eFileId::eRopes },
    { QUrl("https://www.dropbox.com/scl/fi/c50dk7kn5743etsoa696h/scrab.zip?rlkey=ink6gaqjr67hpcwvjqs6rh2uw&st=e8kwj8dr&dl=1"), "scrab.zip", eFileId::eScrab },
    { QUrl("https://www.dropbox.com/scl/fi/lqnpiays5tb3z7xq3q1zs/security_door.zip?rlkey=8tuytn5rnz156t7mg5apuoati&st=oov7vq26&dl=1"), "security_door.zip", eFileId::eSecurityDoor },
    { QUrl("https://www.dropbox.com/scl/fi/gzxlclu7r8qctxjgz3umz/security_orb.zip?rlkey=u2q6gkpkd25hp19y4xsjt1sbz&st=a314g8bi&dl=1"), "security_orb.zip", eFileId::eSecurityOrb },
    { QUrl("https://www.dropbox.com/scl/fi/f2jcvrnv36zy3x24u8le1/slam_doors.zip?rlkey=o02l2x8zj7mvt13mrf4uousmj&st=pq5edkl9&dl=1"), "slam_doors.zip", eFileId::eSlamDoors },
    { QUrl("https://www.dropbox.com/scl/fi/kuhkg7le3cy7avz8q2lkl/slig-AE-no-tubes.zip?rlkey=3uplrgx4kighddxva8bokjo3e&st=o65sygym&dl=1"), "slig-AE-no-tubes.zip", eFileId::eSligAENoTubes },
    { QUrl("https://www.dropbox.com/scl/fi/fcwr7n1ds2j7li3vhmdc1/slig-AE-tubes.zip?rlkey=elbbif44v67v5z13awv2kbcrt&st=04g354qt&dl=1"), "slig-AE-tubes.zip", eFileId::eSligAETubes },
    { QUrl("https://www.dropbox.com/scl/fi/dssmsh8hd4upi1zkfp8i1/slig-AO-no-tubes.zip?rlkey=zioctx5l83u1akny3c7sj3io6&st=fasa6rbi&dl=1"), "slig-AO-no-tubes.zip", eFileId::eSligAONoTubes },
    { QUrl("https://www.dropbox.com/scl/fi/1c8cb0r0h640k1tf92zy0/slig-AO-tubes.zip?rlkey=xmj2g6rwrnsv4nacdindn7cua&st=uekah3ho&dl=1"), "slig-AO-tubes.zip", eFileId::eSligAOTubes },
    { QUrl("https://www.dropbox.com/scl/fi/tegio5bigwgigpb9ddmwx/slig-MO-no-tubes.zip?rlkey=w6w4wqn45b6wxlwr3l4gxuapz&st=8pawn9r0&dl=1"), "slig-MO-no-tubes.zip", eFileId::eSligMONoTubes },
    { QUrl("https://www.dropbox.com/scl/fi/nkrs7jet29zvig5rxodp1/slig-MO-tubes.zip?rlkey=w5kio30mi79jgitjuiihs2814&st=hagokvf1&dl=1"), "slig-MO-tubes.zip", eFileId::eSligMOTubes },
    { QUrl("https://www.dropbox.com/scl/fi/1380xghlwgfuzkyz16cup/slog.zip?rlkey=xriopfw6xek3elgxijd46ftm9&st=5owe1866&dl=1"), "slog.zip", eFileId::eSlog },
    { QUrl("https://www.dropbox.com/scl/fi/dtd055m5781ykfh564ou0/slurg.zip?rlkey=17vwfox0nfntt8470the4nf01&st=52ughgce&dl=1"), "slurg.zip", eFileId::eSlurg },
    { QUrl("https://www.dropbox.com/scl/fi/ts3t58zonjuh3ootn42dn/throwables.zip?rlkey=hv8ga69rrzkdvce9y8vjvkvsl&st=5sb3fz01&dl=1"), "throwables.zip", eFileId::eThrowables },
    { QUrl("https://www.dropbox.com/scl/fi/pomfbgo4fcu71h537blqc/timed_mine.zip?rlkey=9up0lue0s9blbp446mau4kjy6&st=d7eq09z7&dl=1"), "timed_mine.zip", eFileId::eTimedMine },
    { QUrl("https://www.dropbox.com/scl/fi/twg3u9edcewwahp6ikf3a/tortured_mudokon.zip?rlkey=8f9ze0yyosp1r535uokwvoy9t&st=95fxyvir&dl=1"), "tortured_mudokon.zip", eFileId::eTorturedMudokon },
    { QUrl("https://www.dropbox.com/scl/fi/05q1guzt4iue7ckewk6rf/ungrouped.zip?rlkey=tjdi5reya3xx3uwvy3oano36w&st=w4hcnuyl&dl=1"), "ungrouped.zip", eFileId::eUngrouped },
    { QUrl("https://www.dropbox.com/scl/fi/yxmlrbk2qgglh0tj32sv7/uxb.zip?rlkey=kvvuobnuvcz5rpns0hufkevd7&st=lqjxokma&dl=1"), "uxb.zip", eFileId::eUXB },
    { QUrl("https://www.dropbox.com/scl/fi/82d3ctgcyu1kzyciz1arj/lever.zip?rlkey=8hnlji6x9oi62ah0bca677q6z&st=sbf9yhyd&dl=1"), "lever.zip", eFileId::eLever },
    { QUrl("https://www.dropbox.com/scl/fi/sog2cixn2vudplz7w73gp/lifts.zip?rlkey=4gint5x0wmdiygrypl0rfvvfy&st=igoqqpe9&dl=1"), "lifts.zip", eFileId::eLifts },
    { QUrl("https://www.dropbox.com/scl/fi/damvhgvfzjs1lo9l1zssu/trap_doors.zip?rlkey=8ckd2re57jjz09in3i5djgw3h&st=8rz4u3zy&dl=1"), "trap_doors.zip", eFileId::eTrapDoors },
    { QUrl("https://www.dropbox.com/scl/fi/rwdmx69hrhyyg1d9c475x/work_wheel.zip?rlkey=lihnbr9xnvqt2s9r3w8i52md5&st=yq8c6ii8&dl=1"), "work_wheel.zip", eFileId::eWorkWheel },
    { QUrl("https://www.dropbox.com/scl/fi/fieuk5g3w9sjsw28t55k3/slap_lock.zip?rlkey=9ik920dzaeafhx1hrfbhxq1ai&st=vn1acyq8&dl=1"), "slap_lock.zip", eFileId::eSlapLock },
    { QUrl("https://www.dropbox.com/scl/fi/rrvdkflpi7yx14ldw3fdg/bone_bag.zip?rlkey=oub9rgpjfn9r4ilfb899d4ruk&st=i4mwv1fw&dl=1"), "bone_bag.zip", eFileId::eBoneBag },
    { QUrl("https://www.dropbox.com/scl/fi/8d8ohxyol38yzvck198gy/meat_sack.zip?rlkey=9qezqg2f9879lmyy3q1z8vgzl&st=xv46ateq&dl=1"), "meat_sack.zip", eFileId::eMeatSack },
    { QUrl("https://www.dropbox.com/scl/fi/cwrhrz9pwiuwrgvq805jk/AE_cams-001.zip?rlkey=0gyk0cplyd69oygpjm5q1wsku&st=7q5i0hhd&dl=1"), "AE_cams-001.zip", eFileId::eCams },
    { QUrl("https://www.dropbox.com/scl/fi/z3qm6t8jykofi7nr6or1f/AE_cams-002.zip?rlkey=25ce7equqjcpy3fdvlimjceha&st=0gzq4hb0&dl=1"), "AE_cams-002.zip", eFileId::eCams },
    { QUrl("https://www.dropbox.com/scl/fi/w9xytj34l51m89x9h1h2b/AE_cams-003.zip?rlkey=4arfb2hzti0h53phki269pdsj&st=6qa8cyu1&dl=1"), "AE_cams-003.zip", eFileId::eCams },
    { QUrl("https://www.dropbox.com/scl/fi/p1tslh81h9day72gdqo6r/AE_cams-004.zip?rlkey=1wq4gnyu7p2zap57uobjft2yc&st=2u9lr3a8&dl=1"), "AE_cams-004.zip", eFileId::eCams },
};

static QString GetAssetsPath()
{
    return QCoreApplication::applicationDirPath() + "/assets";
}

static QString GetInfoJsonPath()
{
    return QCoreApplication::applicationDirPath() + "/info.json";
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

MainWindow::MainWindow(QWidget* pParent)
    : QMainWindow(pParent)
    , mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    SetSelectedGame(mUi->currentGameComboBox->currentIndex() == 0 ? eGameType::eAO : eGameType::eAE);

    QDir().mkpath(GetAssetsPath());

    setFixedSize(size());

    mAEBackground = QPixmap(":/images/AE.png").scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
    mAOBackground = QPixmap(":/images/AO.png").scaled(size(), Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    mDownloader = new Downloader(this);

    connect(mDownloader, &Downloader::progress, this, [this](const AssetFile& assetFile, qint64 received, qint64 total)
    {
        if (total > 0 && received > 0)
        {
            int progress = static_cast<int>((static_cast<double>(received) / total) * 100.0);
            mUi->downloadProgressBar->setValue(progress);

            double currentMB = received / (1000.0 * 1000.0);
            double totalMB = total / (1000.0 * 1000.0);
            mUi->downloadLabel->setText(QString("%1 (%2 / %3 MB)").arg(assetFile.fileName).arg(currentMB, 0, 'f', 1).arg(totalMB, 0, 'f', 1));
        }
    });

    connect(mDownloader, &Downloader::AllDownloadsFinished, this, [this]()
    {
        qDebug() << "all downloads finished";

        mUi->bigButton->setEnabled(true);
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

    connect(mUi->bigButton, &QPushButton::pressed, this, [this]()
    {
        if (mBigButtonState == eBigButtonState::eWaitForDownload)
        {
            mUi->bigButton->setEnabled(false);
            mUi->currentGameComboBox->setEnabled(false);
            StartDownload(mSelectedGame);
        }
        else if (mBigButtonState == eBigButtonState::eWaitForLaunchGame)
        {
            if (!LaunchGame(mSelectedGame, false))
            {
                QMessageBox::warning(this, "Warning", "Couldn't launch game (make sure the game is installed on steam)");
            }
        }
        else if (mBigButtonState == eBigButtonState::eGameLaunched)
        {
            CloseGame();
            SetBigButtonState(eBigButtonState::eWaitForLaunchGame);
        }
    });

    connect(mUi->discordButton, &QPushButton::pressed, this, []()
    {
        QDesktopServices::openUrl(QUrl("https://discord.com/invite/Ca5uB4JByA"));
    });

    connect(mUi->currentGameComboBox, &QComboBox::currentIndexChanged, this, [this](int index)
    {
        SetSelectedGame(index == 0 ? eGameType::eAO : eGameType::eAE);
    });

    connect(mUi->optionsButton, &QPushButton::pressed, this, [this]()
    {
        auto pDialog = new AssetsDialog(this);
        connect(pDialog, &AssetsDialog::Accepted, this, [this](QList<AssetFile> assetFiles)
        {
            qDebug() << "selected " << assetFiles.size() << "assets for installation";
            mAssetsToInstall = assetFiles;
        });
        pDialog->Populate(sAssetFiles);
        pDialog->exec();
        delete pDialog;
    });
}

MainWindow::~MainWindow()
{
    delete mUi;
}

void MainWindow::paintEvent(QPaintEvent*)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::SmoothPixmapTransform, true);

    int x = (width() - mAOBackground.width()) / 2;
    int y = (height() - mAOBackground.height()) / 2;

    painter.drawPixmap(x, y, mAOBackground);
}

void MainWindow::StartDownload(eGameType gameType)
{
    if (gameType == eGameType::eAE)
    {
        for (const AssetFile& asset : sAssetFiles)
        {
            mDownloader->AddDownload(asset, GetAssetsPath() + "/" + asset.fileName);
        }
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

    for (const AssetFile& asset : sAssetFiles)
    {
        if (asset.fileId == eFileId::eRelive)
        {
            QString zipFilePath = GetAssetsPath() + "/" + asset.fileName;
            int error = zip_extract(zipFilePath.toUtf8().constData(), gameInstallPath.toUtf8().constData(), nullptr, nullptr);
            if (error != 0)
            {
                qDebug() << "Extraction failed for:" << zipFilePath << "with error " << zip_strerror(error);
                return false;
            }

            return true;
        }
    }

    return false;
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

    // relive_data/ae/{level_name}/paths/{path_number}/{camId}.png
    //QString zipCamOutPath = gameInstallPath + "/relive_data/ae/animations";

    for (const AssetFile& asset : sAssetFiles)
    {
        // relive was already installed earlier to run the data conversion first
        if (asset.fileId == eFileId::eRelive)
        {
            continue;
        }

        // skip alt textures for now
        if (asset.fileId == eFileId::eAbeFMV ||
            asset.fileId == eFileId::eSligMOTubes ||
            asset.fileId == eFileId::eSligMONoTubes ||
            asset.fileId == eFileId::eSligAOTubes ||
            asset.fileId == eFileId::eSligAONoTubes ||
            asset.fileId == eFileId::eSligAENoTubes)
        {
            continue;
        }

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

    SetBigButtonState(eBigButtonState::eWaitForLaunchGame);

    mUi->bigButton->setEnabled(true);
    mUi->currentGameComboBox->setEnabled(true);
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
        qDebug() << "Library:" << lib;
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
                SetBigButtonState(eBigButtonState::eWaitForLaunchGame);
            }
        }
    });

    SetBigButtonState(eBigButtonState::eGameLaunched);

    return true;
}

void MainWindow::SetBigButtonState(eBigButtonState state)
{
    if (state == eBigButtonState::eWaitForDownload)
    {
        mUi->bigButton->setText("Download");
    }
    else if (state == eBigButtonState::eWaitForLaunchGame)
    {
        mUi->bigButton->setText("Play");
    }
    else if (state == eBigButtonState::eGameLaunched)
    {
        mUi->bigButton->setText("Close Game");
    }

    mBigButtonState = state;
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
    if (IsGameInstalled(game))
    {
        SetBigButtonState(eBigButtonState::eWaitForLaunchGame);
    }
    else
    {
        SetBigButtonState(eBigButtonState::eWaitForDownload);
    }

    mSelectedGame = game;
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
