#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>

#include "defines.h"
#include "jsoncpp/json/json.h"

#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>


MainWindow::MainWindow(QWidget *parent, std::string user_dir) : QMainWindow(parent),
                                                                ui(new Ui::MainWindow),
                                                                user_dir(user_dir),
                                                                curr_team("") {
    ui->setupUi(this);

    ui->transactions->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    QStringList headers;
    headers.append("Add");
    headers.append("Drop");
    ui->transactions->setHorizontalHeaderLabels(headers);

    // get available teams
    std::string user_config_path = get_user_config_path();
    Json::Value config_root;
    Json::Reader reader;
    std::ifstream stream(user_config_path, std::ifstream::binary);
    bool success = reader.parse(stream, config_root);
    if(success) {
        Json::Value teams = config_root["teams"];
        if(!teams.empty())
            curr_team = teams[0].asString();
        for(const Json::Value & team : teams)
            ui->teams_combobox->addItem(QString::fromUtf8(team.asString().c_str()));
    }
    else {
        fprintf(stderr, "User config file read failed : %s\n", reader.getFormattedErrorMessages().c_str());
        fflush(stderr);
    }
    stream.close();

    // load initial team
    if(!curr_team.empty())
        load_team(curr_team);

    connect(ui->trans_drop_player, &QLineEdit::returnPressed, ui->trans_add_btn, &QPushButton::click);
    connect(ui->logs_combobox, SIGNAL (currentIndexChanged(QString)), this, SLOT (change_log_file(QString)) );
    connect(ui->teams_combobox, SIGNAL (currentIndexChanged(QString)), this, SLOT (change_team(QString)) );

}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::on_trans_up_btn_clicked() {
    move_trans_row(true);
}

void MainWindow::on_trans_down_btn_clicked() {
    move_trans_row(false);
}

void MainWindow::on_trans_remove_btn_clicked() {
    QList<QTableWidgetItem *> selected = ui->transactions->selectedItems();
    if(!selected.empty()) {
        int row = selected.front()->row();
        ui->transactions->removeRow(row);
        int total_rows = ui->transactions->rowCount();
        if(total_rows > 0) {
            if(row > total_rows-1)
                row = total_rows-1;
            ui->transactions->setCurrentCell(row, 0);
            ui->transactions->setFocus();
        }
    }
}

void MainWindow::on_trans_add_btn_clicked() {
    QString add = ui->trans_add_player->text();
    QString drop = ui->trans_drop_player->text();
    if(add.length() > 0 && drop.length() > 0) {
        int row = ui->transactions->rowCount();
        ui->transactions->insertRow(row);
        ui->transactions->setItem(row, 0, new QTableWidgetItem(add));
        ui->transactions->setItem(row, 1, new QTableWidgetItem(drop));

        ui->transactions->setCurrentCell(row, 0);
        ui->transactions->setFocus();

        ui->trans_add_player->clear();
        ui->trans_drop_player->clear();
    }
}

void MainWindow::on_trans_reset_btn_clicked() {
    // clear current transactions
    while(ui->transactions->rowCount() > 0)
        ui->transactions->removeRow(ui->transactions->rowCount()-1);

    // load from file
    load_transactions(curr_team);
    if(ui->transactions->rowCount() > 0) {
        ui->transactions->setCurrentCell(0, 0);
        ui->transactions->setFocus();
    }
}

void MainWindow::on_trans_save_btn_clicked() {
    Json::Value trans_root;

    trans_root["team"] = curr_team;
    trans_root["transactions"] = Json::Value(Json::arrayValue);
    for(int row = 0; row < ui->transactions->rowCount(); row++) {
        Json::Value transaction;
        transaction["add"] = ui->transactions->item(row, 0)->text().toStdString();
        transaction["drop"] = ui->transactions->item(row, 1)->text().toStdString();
        trans_root["transactions"].append(transaction);
    }

    std::ofstream stream;
    stream.open(get_transactions_path(curr_team), std::ofstream::out | std::ofstream::trunc);
    stream << trans_root;
    stream.close();
}

void MainWindow::change_team(const QString & text) {
    std::string new_team = text.toStdString();
    if(new_team.compare(curr_team)) {
        QMessageBox::StandardButton answer = QMessageBox::question(this,
                                                                   "Load New Team",
                                                                   "Are you sure you want to load a new team? Any unsaved changes to transactions will be lost.");
        if(answer == QMessageBox::StandardButton::Yes) {
            curr_team = new_team;
            load_team(new_team);
        }
    }
}

void MainWindow::change_log_file(const QString & text) {
    ui->log->clear();
    load_log_file(curr_team, text.toStdString());
}

void MainWindow::on_logs_refresh_btn_clicked() {
    refresh_logs(curr_team);
}

void MainWindow::load_team(const std::string & team) {
    // clear transactions
    ui->transactions->clearContents();
    ui->transactions->setRowCount(0);

    load_transactions(team);
    if(ui->transactions->rowCount() > 0) {
        ui->transactions->setCurrentCell(0, 0);
        ui->transactions->setFocus();
    }

    refresh_logs(team);
}

void MainWindow::load_transactions(const std::string & team) {
    std::string path = get_transactions_path(team);
    Json::Value trans_root;
    Json::Reader reader;

    std::ifstream stream(path, std::ifstream::binary);
    bool success = reader.parse(stream, trans_root);
    if(!success) {
        fprintf(stderr, "Transactions file read failed : %s\n", reader.getFormattedErrorMessages().c_str());
        fflush(stderr);
        return;
    }

    Json::Value transactions = trans_root["transactions"];
    for(const Json::Value & transaction : transactions) {
        std::string add = transaction["add"].asString();
        std::string drop = transaction["drop"].asString();

        int row = ui->transactions->rowCount();
        ui->transactions->insertRow(row);
        ui->transactions->setItem(row, 0, new QTableWidgetItem(QString::fromUtf8(add.c_str())));
        ui->transactions->setItem(row, 1, new QTableWidgetItem(QString::fromUtf8(drop.c_str())));
    }

    stream.close();
}

void MainWindow::load_log_file(const std::string & team, const std::string & file_name) {
    std::string path = get_logs_dir_path(team) + "/" + file_name;
    FILE * f = fopen(path.c_str(), "r");
    if(!f) {
        fprintf(stderr, "Invalid log file: %s\n", path.c_str());
        fflush(stderr);
    }
    char *line = NULL;
    size_t capacity = 0;
    ssize_t result;
    std::string str = "";
    while( (result = getline(&line, &capacity, f)) != -1) {
        str += std::string(line);

    }
    ui->log->setText(QString::fromUtf8(str.c_str()));
    free(line);
    fclose(f);
}

void MainWindow::refresh_logs(const std::string & team) {
    QString curr = ui->logs_combobox->currentText();

    ui->logs_combobox->clear();

    std::string logs_dir_path = get_logs_dir_path(team);
    struct dirent *dp;
    DIR *dirp = opendir(logs_dir_path.c_str());
    if(dirp) {
        while((dp = readdir(dirp))) {
            if(!strcmp(dp->d_name, ".") || !strcmp(dp->d_name, ".."))
                continue;

            char file_path[strlen(logs_dir_path.c_str()) + strlen(dp->d_name) + 2];
            sprintf(file_path, "%s/%s", logs_dir_path.c_str(), dp->d_name);
            struct stat s;
            if(!stat(file_path, &s) && S_ISREG(s.st_mode))
                ui->logs_combobox->addItem(QString::fromUtf8(dp->d_name));
        }
        closedir(dirp);
    }

    // load file
    if(ui->logs_combobox->count() > 0) {
        int index = ui->logs_combobox->findText(curr);
        if(index == -1)
            index = 0;
        ui->logs_combobox->setCurrentIndex(index);

       change_log_file(ui->logs_combobox->currentText());
    }
}


void MainWindow::move_trans_row(bool up) {
    QList<QTableWidgetItem *> selected = ui->transactions->selectedItems();
    if(!selected.empty()) {
        int row = selected.front()->row();
        int new_row = row;
        int bound = up ? 0 : (ui->transactions->rowCount()-1);
        QTableWidgetItem *orig_add = ui->transactions->takeItem(row, 0);
        QTableWidgetItem *orig_drop = ui->transactions->takeItem(row, 1);
        if(row != bound) {
            new_row = up ? row-1 : row+1;
            QTableWidgetItem *other_add = ui->transactions->takeItem(new_row, 0);
            QTableWidgetItem *other_drop = ui->transactions->takeItem(new_row, 1);
            ui->transactions->setItem(row, 0, other_add);
            ui->transactions->setItem(row, 1, other_drop);
        }
        ui->transactions->setItem(new_row, 0, orig_add);
        ui->transactions->setItem(new_row, 1, orig_drop);

        ui->transactions->setCurrentCell(new_row, 0);
        ui->transactions->setFocus();
    }
}

std::string MainWindow::get_user_config_path() {
    return user_dir + "/" + USER_CONFIG_FILE;
}

std::string MainWindow::get_transactions_path(const std::string & team) {
    return user_dir + "/" + team + "/" + TRANSACTIONS_FILE;
}

std::string MainWindow::get_logs_dir_path(const std::string & team) {
    return user_dir + "/" + team + "/" + "Logs";
}
