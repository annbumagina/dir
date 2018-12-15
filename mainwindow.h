#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "task.h"
#include <QLabel>
#include <QMainWindow>
#include <QString>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_pushButton_clicked();
    void on_removeButton_clicked();
    void update(std::vector< std::vector<QString> >);
    void scan_finished();
    void scan_started();
    void on_cancelButton_clicked();
    void on_select_directory_clicked();

signals:
    void cancel();
    void started(QString);

private:
    Ui::MainWindow *ui;
    QLabel * labelDupes;
    QThread *thread;
    Task *t;
    QString dir = "";
};

#endif // MAINWINDOW_H
