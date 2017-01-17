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
    void on_trans_add_btn_clicked();
    void on_trans_reset_btn_clicked();
    void on_trans_save_btn_clicked();

    void change_team(const QString & text);
    void change_log_file(const QString & text);
    void on_logs_refresh_btn_clicked();

private:
    Ui::MainWindow *ui;

    void load_team(const std::string & team);
    void load_transactions(const std::string & team);
    void load_log_file(const std::string & team, const std::string & file_name);
    void refresh_logs(const std::string & team);
    void move_trans_row(bool up);
    std::string get_user_config_path();
    std::string get_transactions_path(const std::string & team);
    std::string get_logs_dir_path(const std::string & team);

    std::string user_dir;
    std::string curr_team;
};

#endif // MAINWINDOW_H
