#ifndef QTMMAUD_SETTINGSWIDGET_H
#define QTMMAUD_SETTINGSWIDGET_H

#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

namespace Ui {
    class SettingsWidget;
}


class SettingsWidget : public QWidget {
public:
    Q_OBJECT


public:
    explicit SettingsWidget();

    void closeEvent(QCloseEvent *event) override;
    Ui::SettingsWidget *ui;

    QVBoxLayout *layout;
    QLabel *label;
    QLineEdit *lineEdit;
    QPushButton *saveBtn;
public slots:

    void show();

    void hide();

signals:
    void settingsClosed();
};


#endif

