#include "logindialog.h"
#include "ui_logindialog.h"

#include <jsoncpp/json/json.h>
#include "secret.h"

#include <QMessageBox>

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#define DEFAULT_CREATE_MODE 0755

LoginDialog::LoginDialog(QWidget *parent, std::string users_dir, std::string *path) : QDialog(parent),
                                                                                      ui(new Ui::LoginDialog),
                                                                                      users_dir(users_dir),
                                                                                      chosen_path(path) {
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::on_login_btn_clicked() {
    std::string user = ui->username_line->text().toStdString();
    std::string password = password_trans(ui->password_line->text().toStdString());

    std::string user_path = users_dir + std::string("/") + user;
    std::string config_path = user_path + "/config.json";

    struct stat s;
    // user dir NOT found
    if(stat(user_path.c_str(), &s) || !S_ISDIR(s.st_mode)) {
        std::string message = user + " not found. Do you want to create a new user?";
        QMessageBox::StandardButton answer = QMessageBox::question(this,
                                                                   "No User Found",
                                                                   QString::fromUtf8(message.c_str()));

        if(answer == QMessageBox::StandardButton::No) {
            ui->password_line->clear();
            ui->username_line->setFocus();
            ui->username_line->selectAll();
            return;
        }

        int result = mkdir(user_path.c_str(), (mode_t) DEFAULT_CREATE_MODE);
        if(result) {
            perror("Failed to create user dir");
            return;
        }

        std::ofstream config;
        config.open(config_path, std::ofstream::out | std::ofstream::trunc);

        Json::Value root;
        root["password"] = password;
        config << root;
    }
    // returning user
    // need to validate password
    else {
        Json::Value root;
        Json::Reader reader;

        std::ifstream json_stream(config_path, std::ifstream::binary);
        bool success = reader.parse(json_stream, root);
        if(!success) {
            fprintf(stderr, "User config file read failed : %s\n", reader.getFormattedErrorMessages().c_str());
            fflush(stderr);
            return;
        }

        std::string actual = root["password"].asString();
        if(password.compare(actual)) {
            ui->password_line->setFocus();
            ui->password_line->clear();
            return;
        }
    }
    *chosen_path = user_path;
    close();
}
