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

private:
    Ui::MainWindow *ui;

    std::string user_dir;
};

#endif // MAINWINDOW_H
