#include "moditem.h"
#include "ui_moditem.h"
#include <curl/curl.h>
#include <QMessageBox>
#include <QThread>
#include <QMovie>
#include <iostream>
#include <fstream>
#include <QProcess>
#include <windows.h>
#include <filesystem>


ModItem::ModItem(const std::string &modName,
                 const std::string &modVersion,
                 const std::string &modUrl,
                 QWidget *parent) : QWidget(parent), ui(new Ui::ModItem) {
    ui->setupUi(this);
    ui->ModName->setText(tr(modName.c_str()));
    ui->ModVersion->setText(tr(modVersion.c_str()));
    _modUrl = modUrl;
    _modName = modName;


    changeModStatus(ModStatus::Undefined);

    // Set Download Button
    connect(ui->DownloadBtn,
            &QPushButton::released,
            this,
            [this] {
                spinner->start();
                auto *workerThread = new ModDownloadThread(this);
                connect(workerThread, SIGNAL(finished()),
                        workerThread, SLOT(deleteLater()));


                workerThread->start();
            });

    spinner = new WaitingSpinnerWidget(ui->DownloadBtn, false, false);

    // Center horizontally spinner && add 10 left margin
    spinner->move(10, (ui->DownloadBtn->height() / 2 - spinner->lineLength() / 2 - spinner->innerRadius() / 2));

    spinner->start();
}

ModItem::~ModItem() {
    delete ui;
}

void ModItem::setColorTheme(bool darkMode) {
    if (darkMode) {
        ui->frame->setStyleSheet("color: white");
        ui->DownloadBtn->setStyleSheet("background-color:#2E2C38");
    } else {
        ui->frame->setStyleSheet("");
        ui->DownloadBtn->setStyleSheet("");
    }
}

void ModItem::changeModStatus(ModStatus new_status) {
    currentModStatus = new_status;
    const char *modStatusString;
    //ChangeIcon

    switch (currentModStatus) {
        case Downloaded:
            this->ui->DownloadBtn->setIcon(QIcon(":/Resources/Icons/Download_Done.svg"));
            modStatusString = "Downloaded";
            if (spinner->isSpinning()) {
                spinner->stop();
            }
            this->ui->DownloadBtn->setDisabled(true);
            break;
        case Downloading:
            this->ui->DownloadBtn->setIcon(QIcon());
            modStatusString = "Downloading";
            this->ui->DownloadBtn->setDisabled(true);
            break;
        case Not_Downloaded:
            this->ui->DownloadBtn->setIcon(QIcon(":/Resources/Icons/Download.svg"));
            modStatusString = "Download";
            if (spinner->isSpinning()) {
                spinner->stop();
            }
            this->ui->DownloadBtn->setDisabled(false);
            break;
        case Not_Installed:
            this->ui->DownloadBtn->setIcon(QIcon(":/Resources/Icons/Download.svg"));
            modStatusString = "Install";
            if (spinner->isSpinning()) {
                spinner->stop();
            }
            this->ui->DownloadBtn->setDisabled(false);
            break;
        case Installing:
            this->ui->DownloadBtn->setIcon(QIcon());
            modStatusString = "Installing";
            spinner->start();
            this->ui->DownloadBtn->setDisabled(true);
            break;
        case Installed:
            this->ui->DownloadBtn->setIcon(QIcon(":/Resources/Icons/Download_Done.svg"));
            modStatusString = "Installed";
            if (spinner->isSpinning()) {
                spinner->stop();
            }
            this->ui->DownloadBtn->setDisabled(true);
            break;
        case Undefined:
            this->ui->DownloadBtn->setIcon(QIcon(":/Resources/Icons/Question_Mark.svg"));
            modStatusString = "N/A";
            this->ui->DownloadBtn->setDisabled(false);
            break;
    }

    this->ui->DownloadBtn->setText(QString(tr(modStatusString)));
}

void ModItem::applyForgeColorTheme(bool isInDarkMode) {
    if (ModItem::_modName.substr(0, 5) == "forge") {
        this->ui->frame->setStyleSheet("QFrame{background-color: #26303d} QLabel{color: #de9e59}");
        if (isInDarkMode) {
            this->ui->DownloadBtn->setStyleSheet("color: white; background-color:#2E2C38; border-radius: 5px; border: 1px solid #3E3C49}");
        }
    }
}

void ModItem::InstallForge() {
    std::string appdataRoaming = getenv("APPDATA");
    std::string modLocation = appdataRoaming + R"(\.minecraft\mods\)";
    std::string command = modLocation + ModItem::_modName;
    ModItem::changeModStatus(ModStatus::Installing);
    system(command.c_str());
    ModItem::changeModStatus(ModStatus::Installed);
}

ModItem::ForgeVersionObj ModItem::GetForgeVersionsInstalled() {

    std::string appdataRoaming = getenv("APPDATA");

    std::string versions_file_path = appdataRoaming + "\\.minecraft\\launcher_profiles.json";
    std::ifstream versions_file(versions_file_path);

    std::string forgeVersionInstalled = "";
    if (versions_file.good()) {
        json versions_file_json = json::parse(versions_file);
        for (auto &elem: versions_file_json["profiles"]) {
            if (elem["name"] == "forge") {
                forgeVersionInstalled = elem["lastVersionId"];
                break;
            }
        }
    }
    ForgeVersionObj v = *new ForgeVersionObj;
    if (!forgeVersionInstalled.empty()) {
        v.minecraft_version = forgeVersionInstalled.substr(forgeVersionInstalled.find_last_of(R"(\\)") + 1);
        v.minecraft_version = v.minecraft_version.substr(0, v.minecraft_version.find('-'));
        v.forge_version = forgeVersionInstalled.substr(forgeVersionInstalled.find_last_of('-') + 1);

    }
    return v;
}


void ModItem::ModDownloadThread::run() {
    if (modItem->currentModStatus == ModStatus::Not_Installed) {
        modItem->InstallForge();
    } else {
        modItem->changeModStatus(ModStatus::Downloading);
        std::string appdataRoaming = getenv("APPDATA");
        std::string modLocation = appdataRoaming + R"(\.minecraft\mods\)";
        CURL *curl;
        CURLcode res;
        std::string readBuffer;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, modItem->_modUrl.c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            if (res == CURLE_OK) {
                std::ofstream file;
                file.open(modLocation + modItem->_modName, std::ios::binary);
                file << readBuffer;
                file.close();

                if (modItem->_modName.substr(0, 5) == "forge") {
                    auto currentForgeVInstalled = modItem->GetForgeVersionsInstalled();
                    std::string modForgeVersion = modItem->_modName.substr(0, modItem->_modName.find_last_of('-'));
                    modForgeVersion = modForgeVersion.substr(modForgeVersion.find_last_of('-') + 1);

                    if (currentForgeVInstalled.forge_version == modForgeVersion) {
                        modItem->changeModStatus(ModStatus::Installed);
                    } else {
                        modItem->changeModStatus(ModStatus::Not_Installed);
                    }
                } else {
                    modItem->changeModStatus(ModStatus::Downloaded);
                }
            } else {
                modItem->changeModStatus(ModStatus::Not_Downloaded);
            }
        }
    }

}
