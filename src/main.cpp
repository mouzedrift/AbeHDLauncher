#include "mainwindow.hpp"
#include <QApplication>
#include <QMessageBox>
#include <QFontDatabase>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    
    const char* customFontFiles[] = {"NewPortLandOpti-Regular.otf", "URW Contemporary Brush W01 Bd.ttf"};
    for (const char* fontFile : customFontFiles)
    {
        QString fontPath = QString(":/fonts/%1").arg(fontFile);
        int fontId = QFontDatabase::addApplicationFont(fontPath);
        if (fontId == -1)
        {
            qDebug() << "custom font failed to load:" << fontPath;
        }
    }

    QString aeInstallPath = MainWindow::FindGameInstallPath(eGameType::eAE);
    QString aoInstallPath = MainWindow::FindGameInstallPath(eGameType::eAO);
    if (aeInstallPath.isEmpty() && aoInstallPath.isEmpty())
    {
        QMessageBox::critical(nullptr, "Error", "Couldn't find AO or AE game path.\nPlease install the game through steam and try again.");
        return 0;
    }

    MainWindow mainWindow;
    mainWindow.show();
    return app.exec();
}