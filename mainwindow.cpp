#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent, std::string user_dir) : QMainWindow(parent),
                                                                ui(new Ui::MainWindow),
                                                                user_dir(user_dir) {
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
    delete ui;
}
