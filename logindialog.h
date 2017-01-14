#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent, std::string users_dir, std::string *path);
    ~LoginDialog();
public slots:
    void on_login_btn_clicked();

private:
    Ui::LoginDialog *ui;

    std::string users_dir;
    std::string *chosen_path;
};

#endif // LOGINDIALOG_H
