#include "logindialog.h"
#include "mainwindow.h"
#include "jsoncpp/json/json.h"

#include <QApplication>
#include <QMessageBox>
#include <QDir>

#include <iostream>
#include <fstream>
#include <stdio.h>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // load configuration settings
    Json::Value root;
    Json::Reader reader;

    std::ifstream config_stream("config.json", std::ifstream::binary);
    bool success = reader.parse(config_stream, root);
    if(!success) {
        QMessageBox::critical(NULL,
                              "Configuration File Not Found",
                              "No configuration file found. Add config.json to the directory the executable is located");
        a.quit();
        return -1;
    }

    std::string users_dir = root["users_dir"].asString();

    std::string path = "";
    LoginDialog w1(NULL, users_dir, &path);
    w1.exec();

    if(!path.compare("")) {
        a.quit();
        return 0;
    }

    MainWindow w2(NULL, path);
    w2.show();

    // TODO: fix bug where a.exec() hangs up if MainWindow is not opened

    return a.exec();
}
