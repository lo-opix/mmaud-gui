#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "moditem.h"
#include <QPushButton>
#include <QGridLayout>
#include <QGraphicsBlurEffect>
#include <filesystem>
#include "secrets.h"
#include <sys/stat.h>


using namespace std;
namespace fs = std::filesystem;


MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow) {
    ui->setupUi(this);


    auto *ModsWidget = new QWidget();
    auto *ModsWidgetLayout = new QVBoxLayout();
    ModsWidgetLayout->setAlignment(Qt::AlignTop);
    ModsWidget->setLayout(ModsWidgetLayout);

    ui->scrollArea->setWidget(ModsWidget);
    MainPageSpinner.setParent(this);
    connect(ui->ReloadMods, &QPushButton::released, this, &MainWindow::reloadMods);
    connect(ui->LaunchMinecraftBTN, &QPushButton::released, this, &MainWindow::launchMinecraft);

    MainWindow::InitModsPage();
}

void MainWindow::launchMinecraft() {
    system(R"(C:\Windows\explorer.exe shell:appsFolder\Microsoft.4297127D64EC6_8wekyb3d8bbwe!Minecraft)");
    MainWindow::~MainWindow();
}

void MainWindow::reloadMods() {
    qDeleteAll(ui->scrollArea->widget()->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    MainWindow::InitModsPage();
}

void MainWindow::InitModsPage(){
    auto *dl_Thread = new DownloadRemoteIndexThread(this, GoogleApiKey.c_str());
    connect(dl_Thread, &DownloadRemoteIndexThread::dl_finished, this, &MainWindow::postRemoteIndexDownload);
    connect(dl_Thread, &DownloadRemoteIndexThread::finished, dl_Thread, &QObject::deleteLater);

    this->ShowLoadingScreen(true);
    dl_Thread->start();
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::postRemoteIndexDownload(bool isOk) {
    this->ShowLoadingScreen(true);
    MainPageSpinner.start();

    auto local_index = GetLocalIndex();
    auto compareThread = new FindModsToDownloadThread(this, local_index);
    connect(compareThread, &FindModsToDownloadThread::modsFounded, this, &MainWindow::postFindModsToDownload);
    compareThread->start();

}

void MainWindow::ShowLoadingScreen(bool show) {
    if (show) {
        if (ui->frame->graphicsEffect()) {
            ui->frame->graphicsEffect()->setEnabled(true);
            MainPageSpinner.start();
        } else {
            MainPageSpinner.start();
            auto blur = new QGraphicsBlurEffect(this);
            blur->setBlurRadius(10);
            ui->frame->setGraphicsEffect(blur);
        }

    } else {
        MainPageSpinner.stop();
        if (ui->frame->graphicsEffect()) {
            ui->frame->graphicsEffect()->setEnabled(false);
        }
    }
}


std::vector<std::string> MainWindow::GetLocalIndex() {
    vector<std::string> mod_files;

    std::string appdataRoaming = getenv("APPDATA");
    std::string modsFolder = appdataRoaming + R"(\.minecraft\mods\)";

    struct stat info;
    if (stat(modsFolder.c_str(), &info) != 0) {
        fs::create_directory(modsFolder.c_str());
    }

    for (const auto &entry: fs::directory_iterator(modsFolder)) {
        std::string file_name = entry.path().generic_string();


        file_name.erase(0, modsFolder.length());

        std::string fileExtention = file_name;
        if (fileExtention.substr(fileExtention.find_last_of(".") + 1) == "jar") //file has the extention asked
        {
            mod_files.push_back(file_name);
        }
    }
    return mod_files;
}


void MainWindow::postFindModsToDownload(std::vector<MOD> modsToDl, std::vector<std::string> modsAlreadyDl) {

    for (auto &mod: modsToDl) {
        std::string mod_url = "https://www.googleapis.com/drive/v3/files/" + mod.id + "?alt=media&key=" + GoogleApiKey;
        auto *modWidget = new ModItem(mod.name, "V??", mod_url);
        modWidget->applyForgeColorTheme();
        ModItem::ForgeVersionObj ForgeV;

        if (mod.name.substr(0, 5) == "forge") { //Check if the mod is forge
            ForgeV = modWidget->GetForgeVersionsInstalled();
            std::string thisForgeV = mod.name.substr(0, mod.name.find_last_of('-'));
            thisForgeV.substr(thisForgeV.find_last_of('-') + 1);
            if (thisForgeV == ForgeV.forge_version) {
                modWidget->changeModStatus(ModItem::ModStatus::Installed);
            } else {
                modWidget->changeModStatus(ModItem::ModStatus::Not_Installed);
            }

        } else {
            modWidget->changeModStatus(ModItem::ModStatus::Not_Downloaded);
        }

        auto *scrollWidget = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
        scrollWidget->addWidget(modWidget);
    }
    for (auto &mod: modsAlreadyDl) {
        auto *modWidget = new ModItem(mod);
        ModItem::ForgeVersionObj ForgeV;


        if (mod.substr(0, 5) == "forge") { //Check if the mod is forge
            ForgeV = modWidget->GetForgeVersionsInstalled();
            std::string thisForgeV = mod.substr(0, mod.find_last_of('-'));
            thisForgeV = thisForgeV.substr(thisForgeV.find_last_of('-') + 1);
            if (thisForgeV == ForgeV.forge_version) {
                modWidget->changeModStatus(ModItem::ModStatus::Installed);
            } else {
                modWidget->changeModStatus(ModItem::ModStatus::Not_Installed);
            }

        } else {
            modWidget->changeModStatus(ModItem::ModStatus::Downloaded);
        }


        modWidget->applyForgeColorTheme();
        auto *scrollWidget = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
        scrollWidget->addWidget(modWidget);
    }


    this->ShowLoadingScreen(false);
}
