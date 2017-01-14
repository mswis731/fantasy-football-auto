#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent, std::string user_dir);
    ~MainWindow();

public slots:
    void on_trans_up_btn_clicked();
    void on_trans_down_btn_clicked();
    void on_trans_remove_btn_clicked();

private:
    Ui::MainWindow *ui;

    void load_team(const std::string & team);
    void load_transactions(const std::string & team);
    void move_trans_row(bool up);

    std::string user_dir;
    std::string curr_team;
};

#endif // MAINWINDOW_H
