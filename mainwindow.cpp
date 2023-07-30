#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "moditem.h"
#include <QPushButton>
#include <QGridLayout>
#include <QGraphicsBlurEffect>
#include <filesystem>
#include "secrets.h"
#include <sys/stat.h>
#include <QLineEdit>


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
    connect(ui->ColorThemeBTN, &QPushButton::released, this, &MainWindow::switchColorTheme);
    connect(ui->SettingsBtn, &QPushButton::released, this, &MainWindow::openSettings);


    MainWindow::InitModsPage();
}

std::string MainWindow::getFolderId() {
    ifstream file;
    std::string appDataRoaming = getenv("APPDATA");
    file.open(appDataRoaming + "\\.minecraft\\mmaud-folder-id.txt");

    std::string folder_id;
    if (file.good()) {
        getline(file, folder_id);
        file.close();
        return folder_id;
    } else {
        file.close();
        return folder_id;
    }
}

void MainWindow::setFolderId(const char *folder_id) {
    std::string appDataRoaming = getenv("APPDATA");
    std::string fullPath = appDataRoaming + "\\.minecraft\\mmaud-folder-id.txt";

    std::ofstream file(fullPath);
    file.write(folder_id, strlen(folder_id));
    file.close();
}

void MainWindow::openSettings() {
    auto *settingsWidget = new QWidget();
    auto *layout = new QVBoxLayout();
    settingsWidget->setLayout(layout);
    auto *label = new QLabel(tr("Set your Folder id here"));

    auto currentFolderId = MainWindow::getFolderId();

    auto *lineEdit = new QLineEdit(tr(currentFolderId.c_str()));
    auto *saveBtn = new QPushButton(tr("Save"));
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(saveBtn);


    MainWindow::ShowLoadingScreen(true, isInDarkMode);

    settingsWidget->show();

    connect(saveBtn, &QPushButton::released, this, [lineEdit, this] {
        MainWindow::setFolderId(lineEdit->text().toStdString().c_str());
        MainWindow::reloadMods();
    });
    connect(saveBtn, &QPushButton::released, settingsWidget, &QWidget::hide);
}

void MainWindow::switchColorTheme() {
    if (!isInDarkMode) {
        isInDarkMode = true;
        setStyleSheet(
                "QWidget#centralwidget {background-color: #16151a} QPushButton {background-color:#2E2C38; border-radius: 5px; border: 1px solid #3E3C49}");
        ui->label_2->setStyleSheet("color: white");
        ui->LaunchMinecraftBTN->setStyleSheet("color: white;");
        ui->ReloadMods->setStyleSheet("color: white;");
        ui->scrollArea->setStyleSheet("background-color: #16151a");
        ui->ColorThemeBTN->setIcon(QIcon(":/Resources/Icons/Contrast-white.svg"));
        ui->SettingsBtn->setIcon(QIcon(":/Resources/Icons/Settings-white.svg"));

        auto *scrollWidget = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());

        for (int i = 0; i < scrollWidget->count(); ++i) {
            auto *modWidget = dynamic_cast<ModItem *>(scrollWidget->itemAt(i)->widget());
            modWidget->setColorTheme(true);
            modWidget->applyForgeColorTheme(isInDarkMode);
        }

    } else {
        isInDarkMode = false;
        setStyleSheet(
                "QWidget#centralwidget {background-color: #F4F4F4} QPushButton {background-color: #EAEAEA; border-radius: 5px; border: 1px solid #CCCCCC} *{color: #111111}");
        ui->label_2->setStyleSheet("");
        ui->LaunchMinecraftBTN->setStyleSheet("");
        ui->ReloadMods->setStyleSheet("");
        ui->scrollArea->setStyleSheet("");
        ui->ColorThemeBTN->setIcon(QIcon(":/Resources/Icons/Contrast.svg"));
        ui->SettingsBtn->setIcon(QIcon(":/Resources/Icons/Settings.svg"));
        auto *scrollWidget = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());

        for (int i = 0; i < scrollWidget->count(); ++i) {
            auto *modWidget = dynamic_cast<ModItem *>(scrollWidget->itemAt(i)->widget());
            modWidget->setColorTheme(false);
            modWidget->applyForgeColorTheme(isInDarkMode);
        }
    }
}

void MainWindow::launchMinecraft() {
    system(R"(C:\Windows\explorer.exe shell:appsFolder\Microsoft.4297127D64EC6_8wekyb3d8bbwe!Minecraft)");
    MainWindow::~MainWindow();
}

void MainWindow::reloadMods() {
    qDeleteAll(ui->scrollArea->widget()->findChildren<QWidget *>(QString(), Qt::FindDirectChildrenOnly));
    MainWindow::InitModsPage();
}

void MainWindow::InitModsPage() {
    if (this->getFolderId().empty()) {
        QMessageBox::warning(this, tr("Warning"),
                             tr("Warning: Folder Id is not set!\nSet it by clicking on the settings button"));
    }
    auto *dl_Thread = new DownloadRemoteIndexThread(this, GoogleApiKey.c_str(), this->getFolderId());
    connect(dl_Thread, &DownloadRemoteIndexThread::dl_finished, this, &MainWindow::postRemoteIndexDownload);
    connect(dl_Thread, &DownloadRemoteIndexThread::finished, dl_Thread, &QObject::deleteLater);

    this->ShowLoadingScreen(true, isInDarkMode);
    dl_Thread->start();
}

MainWindow::~MainWindow() {
    delete ui;
}


void MainWindow::postRemoteIndexDownload(bool isOk) {
    this->ShowLoadingScreen(true, isInDarkMode);
    MainPageSpinner.start();

    auto local_index = GetLocalIndex();
    auto compareThread = new FindModsToDownloadThread(this, local_index);
    connect(compareThread, &FindModsToDownloadThread::modsFounded, this, &MainWindow::postFindModsToDownload);
    compareThread->start();

}

void MainWindow::ShowLoadingScreen(bool show, bool isInDarkMode) {
    if (show) {
        if (ui->frame->graphicsEffect()) {
            ui->frame->graphicsEffect()->setEnabled(true);
            MainPageSpinner.start();
            this->setDisabled(true);
        } else {
            MainPageSpinner.start();
            auto blur = new QGraphicsBlurEffect(this);
            blur->setBlurRadius(10);
            ui->frame->setGraphicsEffect(blur);
            this->setDisabled(true);
        }

    } else {
        MainPageSpinner.stop();
        if (ui->frame->graphicsEffect()) {
            ui->frame->graphicsEffect()->setEnabled(false);
        }
        this->setDisabled(false);
    }
    if (isInDarkMode) {
        MainPageSpinner.setColor("white");
    } else {
        MainPageSpinner.setColor("black");
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
        modWidget->applyForgeColorTheme(isInDarkMode);
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
        modWidget->setColorTheme(isInDarkMode);
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


        modWidget->applyForgeColorTheme(isInDarkMode);
        auto *scrollWidget = dynamic_cast<QVBoxLayout *>(ui->scrollArea->widget()->layout());
        scrollWidget->addWidget(modWidget);
        modWidget->setColorTheme(isInDarkMode);
    }


    this->ShowLoadingScreen(false, isInDarkMode);
}
