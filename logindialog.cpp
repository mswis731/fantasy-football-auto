#include "logindialog.h"
#include "ui_logindialog.h"

#include "defines.h"
#include <jsoncpp/json/json.h>
#include "secret.h"

#include <QMessageBox>

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

LoginDialog::LoginDialog(QWidget *parent, std::string users_dir, std::string *user_path_p) : QDialog(parent),
                                                                                             ui(new Ui::LoginDialog),
                                                                                             users_dir(users_dir),
                                                                                             user_path_ptr(user_path_p) {
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint | Qt::WindowMinimizeButtonHint);

    ui->incorrect_password_label->setVisible(false);
}

LoginDialog::~LoginDialog() {
    delete ui;
}

void LoginDialog::on_login_btn_clicked() {
    ui->incorrect_password_label->setVisible(false);

    std::string user = ui->username_line->text().toStdString();
    std::string password = password_trans(ui->password_line->text().toStdString());

    std::string user_path = users_dir + std::string("/") + user;
    std::string config_path = user_path + "/" + USER_CONFIG_FILE;

    Json::Value config_root;
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

        config_root["username"] = user;
        config_root["password"] = password;
        config_root["teams"] = Json::Value(Json::arrayValue);
        config << config_root;
        config.close();
    }
    // returning user
    // need to validate password
    else {
        Json::Reader reader;

        std::ifstream stream(config_path, std::ifstream::binary);
        bool success = reader.parse(stream, config_root);
        if(!success) {
            fprintf(stderr, "User config file read failed : %s\n", reader.getFormattedErrorMessages().c_str());
            fflush(stderr);
            return;
        }

        std::string actual = config_root["password"].asString();
        stream.close();
        if(password.compare(actual)) {
            ui->password_line->setFocus();
            ui->password_line->clear();
            ui->incorrect_password_label->setVisible(true);
            return;
        }
    }
    *user_path_ptr = user_path;
    close();
}
