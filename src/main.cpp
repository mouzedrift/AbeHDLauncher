#include "mainwindow.hpp"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

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