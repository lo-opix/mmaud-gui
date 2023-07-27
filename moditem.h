#ifndef MODITEM_H
#define MODITEM_H

#include <QWidget>
#include "ThirdParty/waitingspinnerwidget.h"
#include <QThread>
#include <cstdio>
#include <nlohmann/json.hpp>
using json = nlohmann::json;


namespace Ui {
    class ModItem;
}

class ModItem : public QWidget {
Q_OBJECT


public:

    enum ModStatus {
        Downloaded = 0, Downloading, Not_Downloaded, Not_Installed, Installing, Installed, Undefined
    };
    ModStatus currentModStatus = ModStatus::Undefined;
    std::string _modName;
    std::string _modUrl;
    WaitingSpinnerWidget *spinner;

    void applyForgeColorTheme();


    explicit ModItem(const std::string &modName = "modName",
                     const std::string &modVersion = "V??.??",
                     const std::string &modUrl = "https://",
                     QWidget *parent = nullptr);

    ~ModItem() override;


    struct ForgeVersionObj {
        std::string minecraft_version;
        std::string forge_version;
    };


    void InstallForge();

    ForgeVersionObj GetForgeVersionsInstalled();


public slots:

    void changeModStatus(ModItem::ModStatus new_status);


private:
    Ui::ModItem *ui;


    class ModDownloadThread : public QThread {
    public :
        ModItem *modItem;

        explicit ModDownloadThread(ModItem *remoteModItem) {
            modItem = remoteModItem;
        }

        void run() override;

    private:
        static size_t WriteCallback(void *contents, size_t size, size_t nmemb, void *userp) {
            ((std::string *) userp)->append((char *) contents, size * nmemb);
            return size * nmemb;
        }
    };


};

#endif // MODITEM_H
