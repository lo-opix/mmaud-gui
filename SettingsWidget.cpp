
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include "SettingsWidget.h"
#include "mainwindow.h"

SettingsWidget::SettingsWidget() : QWidget(){
    layout = new QVBoxLayout();
    this->setLayout(layout);
    label = new QLabel(tr("Set your Folder id here"));

    auto currentFolderId = MainWindow::getFolderId();

    lineEdit = new QLineEdit(tr(currentFolderId.c_str()));
    saveBtn = new QPushButton(tr("Save"));
    layout->addWidget(label);
    layout->addWidget(lineEdit);
    layout->addWidget(saveBtn);
    connect(saveBtn, &QPushButton::released, this, [this]{this->hide();});

}

void SettingsWidget::closeEvent(QCloseEvent *event) {
    this->hide();
    QWidget::closeEvent(event);
    emit settingsClosed();
}

void SettingsWidget::show() {
    QWidget::show();
}

void SettingsWidget::hide() {
    QWidget::hide();
}
