#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ThirdParty/waitingspinnerwidget.h"
#include <QThread>
#include <curl/curl.h>
#include <QMessageBox>
#include <utility>
#include <fstream>
#include "nlohmann/json.hpp"
#include "moditem.h"

using json = nlohmann::json;

namespace fs = std::filesystem;


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow() override;

    void ShowLoadingScreen(bool show);

    struct MOD {
        std::string name;
        std::string id;
    };

private:
    Ui::MainWindow *ui;

    WaitingSpinnerWidget MainPageSpinner = new WaitingSpinnerWidget();

    std::vector<std::string> GetLocalIndex();

    bool isInDarkMode = false;


public slots:

    void postRemoteIndexDownload(bool isOk);

    void postFindModsToDownload(std::vector<MOD> modsToDl, std::vector<std::string> modsAlreadyDl);

    void InitModsPage();

    void reloadMods();

    void launchMinecraft();

    void switchColorTheme();
};

class DownloadRemoteIndexThread : public QThread {
Q_OBJECT

public :
    MainWindow *mW;
    const char *apiKey;
    std::string folderId = "1OoRyW6JkPLGTeDtUBnU11eLYvv3byKNm";

    explicit DownloadRemoteIndexThread(MainWindow *remoteModItem, const char *remoteApiKey) {
        mW = remoteModItem;
        apiKey = remoteApiKey;
    }

    void run() override {
        CURL *curl;
        CURLcode res;
        std::string appdataRoaming = getenv("APPDATA");
        appdataRoaming = appdataRoaming.substr(0, appdataRoaming.find_last_of('\\'));
        std::string modLocation = appdataRoaming + R"(\Local\Temp\mmaud-remote_index.temp)";

        std::string index_url =
                "https://www.googleapis.com/drive/v3/files?q='" + folderId + "'+in+parents&key=" + apiKey;

        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, index_url.c_str());

            /* Use HTTP/3 but fallback to earlier HTTP if necessary */
            curl_easy_setopt(curl, CURLOPT_HTTP_VERSION,(long) CURL_HTTP_VERSION_3);

            FILE *out = fopen(modLocation.c_str(), "w");

            if (out == nullptr) {
                std::string messageContent = "Can't create a file in ";
                messageContent += modLocation;
                QMessageBox::critical(mW, tr("MmaudQT - Error"), tr(messageContent.c_str()));
            } else {
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, out);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                /* Perform the request, res will get the return code */
                curl_easy_perform(curl);
                /* Check for errors */
                if (res != CURLE_OK) {
                    emit dl_finished(false);
                } else {
                    emit dl_finished(true);
                }


                /* always cleanup */
                curl_easy_cleanup(curl);
                fclose(out);
            }
        }
    };

private:
    static size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream) {
        size_t written = fwrite(ptr, size, nmemb, (FILE *) stream);
        return written;
    }

signals:

    void dl_finished(bool isOk);
};


class [[maybe_unused]] FindModsToDownloadThread : public QThread {
Q_OBJECT

public:
    explicit FindModsToDownloadThread(MainWindow *mainWindow, std::vector<std::string> local_index) {
        _local_index = std::move(local_index);
        _mW = mainWindow;
    }


    MainWindow *_mW;
    std::vector<std::string> _local_index;
    std::string appdata = getenv("APPDATA");
    std::string remote_index_file_location = appdata.substr(0, appdata
            .find_last_of('\\')) + R"(\Local\Temp\mmaud-remote_index.temp)";

    void run() override {
        //GetRemoteIndexFromJson
        std::ifstream remote_index_file(remote_index_file_location);
        json remote_index_json = json::parse(remote_index_file);

        std::vector<MainWindow::MOD> remote_mods;

        for (auto &elem: remote_index_json["files"]) {
            MainWindow::MOD temp_mod = *new MainWindow::MOD;
            temp_mod.id = elem["id"];
            temp_mod.name = elem["name"];
            remote_mods.push_back(temp_mod);
        }

        std::vector<MainWindow::MOD> mods_to_download;
        std::vector<std::string> mods_already_downloaded;

        for (auto &r_mod: remote_mods) {
            bool isModInLocal = false;

            for (auto &l_mod: _local_index) {
                if (l_mod == r_mod.name) {
                    isModInLocal = true;
                }
            }

            if (!isModInLocal) {
                mods_to_download.push_back(r_mod);
            } else {
                mods_already_downloaded.push_back(r_mod.name);
            }
        }

        emit modsFounded(mods_to_download, mods_already_downloaded);
    };
signals:

    void modsFounded(std::vector<MainWindow::MOD> modsToDl, std::vector<std::string> modsAlreadyDl);

};

#endif // MAINWINDOW_H
