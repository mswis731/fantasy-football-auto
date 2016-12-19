#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::Dialog | Qt::WindowCloseButtonHint);
    setWindowTitle("Log In");
}

LoginDialog::~LoginDialog()
{
    delete ui;
}
