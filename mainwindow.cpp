#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "task.h"
#include <QDir>
#include <QDirIterator>
#include <QIODevice>
#include <QTextStream>
#include <QFile>
#include <algorithm>
#include <QThread>
#include <QTreeWidget>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::update(std::vector< std::vector<QString> > vs) {
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (size_t i = 0; i < vs.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, vs[i][0]);
        item->setText(1, QString::number(vs[i].size()));
        for (size_t j = 0; j < vs[i].size(); j++) {
            QTreeWidgetItem *child_item = new QTreeWidgetItem();
            child_item->setText(0, vs[i][j]);
            item->addChild(child_item);
        }
    }

}

void MainWindow::on_pushButton_clicked()
{
    QString text = ui->lineEdit->text();

    QThread *thread = new QThread;
    Task *t = new Task(text);
    t->moveToThread(thread);
    connect(t, SIGNAL(send(std::vector< std::vector<QString> >)), this, SLOT(update(std::vector< std::vector<QString> >)));
    connect(thread, SIGNAL(started()), t, SLOT(doWork()));
    thread->start();
}

void MainWindow::on_removeButton_clicked()
{

}
