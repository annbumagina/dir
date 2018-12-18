#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "task.h"
#include <QLabel>
#include <QMainWindow>
#include <QString>
#include <memory>
#include <QElapsedTimer>

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
    void finished(QString);
    void scan_started();
    void on_cancelButton_clicked();
    void on_select_directory_clicked();
    void on_expand_clicked();
    void on_collapse_clicked();
    void on_clear_clicked();

    void on_select_clicked();

signals:
    void cancel();
    void started(QString);
    void remove(std::vector<QString>);

private:
    Ui::MainWindow *ui;
    QLabel * labelDupes;
    std::unique_ptr<QThread> thread;
    Task *t;
    QString dir = "";
    bool removing = false;
    QElapsedTimer *timer;
};

#endif // MAINWINDOW_H
