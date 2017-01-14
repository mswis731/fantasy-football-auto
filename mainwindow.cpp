#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "defines.h"
#include "jsoncpp/json/json.h"

#include <iostream>
#include <fstream>

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
    std::string user_config_path = user_dir + "/" + USER_CONFIG_FILE;
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
        //ui->teams_combobox->setCurrentIndex(0);
    }
    else {
        fprintf(stderr, "User config file read failed : %s\n", reader.getFormattedErrorMessages().c_str());
        fflush(stderr);
    }

    // load initial team
    if(!curr_team.empty())
        load_team(curr_team);
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

void MainWindow::load_team(const std::string & team) {
    load_transactions(team);
    if(ui->transactions->rowCount() > 0) {
        ui->transactions->setCurrentCell(0, 0);
        ui->transactions->setFocus();
    }
}

void MainWindow::load_transactions(const std::string & team) {
    std::string path = user_dir + "/" + team + "/" + TRANSACTIONS_FILE;
    Json::Value trans_root;
    Json::Reader reader;

    std::ifstream trans_stream(path, std::ifstream::binary);
    bool success = reader.parse(trans_stream, trans_root);
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
