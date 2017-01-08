#include "logindialog.h"
#include "ui_logindialog.h"

#include <stdio.h>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::WindowCloseButtonHint);

    connect(ui->password_line, &QLineEdit::returnPressed, ui->login_btn, &QPushButton::click);
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::on_login_btn_clicked() {
}
