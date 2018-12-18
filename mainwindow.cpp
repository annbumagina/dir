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
#include <QDebug>
#include <QCommonStyle>
#include <QFileDialog>
#include <QTreeWidgetItemIterator>
#include <memory>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    thread(new QThread)
{
    ui->setupUi(this);

    QCommonStyle style;
    ui->select_directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    connect(ui->actionScan_Directory, &QAction::triggered, this, &MainWindow::on_select_directory_clicked);
    ui->lineEdit->setReadOnly(true);

    labelDupes = new QLabel(ui->statusBar);
    labelDupes->setAlignment((Qt::AlignRight));
    labelDupes->setMinimumSize(labelDupes->sizeHint());

    t = new Task;
    timer = new QElapsedTimer;
    t->moveToThread(thread.get());
    connect(t, SIGNAL(send(std::vector< std::vector<QString> >)), this, SLOT(update(std::vector< std::vector<QString> >)));
    connect(this, SIGNAL(started(QString)), t, SLOT(doWork(QString)));
    connect(this, SIGNAL(remove(std::vector<QString>)), t, SLOT(remove(std::vector<QString>)));
    connect(t, SIGNAL(scan_started()), this, SLOT(scan_started()));
    connect(t, SIGNAL(finished(QString)), this, SLOT(finished(QString)));
    connect(this, SIGNAL(cancel()), t, SLOT(cancel()), Qt::DirectConnection);
    thread->start();
}

MainWindow::~MainWindow()
{
    emit cancel();
    thread->quit();
    thread->wait();
    delete t;
    delete ui;
}

void MainWindow::update(std::vector< std::vector<QString> > vs) {
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    for (size_t i = 0; i < vs.size(); i++) {
        QTreeWidgetItem *item = new QTreeWidgetItem(ui->treeWidget);
        std::sort(vs.begin(), vs.end());
        item->setText(0, vs[i][0]);
        item->setText(1, QString::number(vs[i].size()));
        for (size_t j = 0; j < vs[i].size(); j++) {
            QTreeWidgetItem *child_item = new QTreeWidgetItem();
            child_item->setText(0, vs[i][j]);
            child_item->setCheckState(2, Qt::Unchecked);
            item->addChild(child_item);
        }
    }
}

void MainWindow::finished(QString message) {
    if (message == "finished")
        message = message + " " + QString::number(timer->elapsed());
    labelDupes->setText(message);
    dir = "";
    removing = false;
}

void MainWindow::scan_started() {
    timer->start();
    labelDupes->setText("doing");
    ui->treeWidget->clear();
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    QString text = ui->lineEdit->text();

    if (dir == "") {
        dir = text;
        emit started(text);
    } else if (dir != text) {
        emit cancel();
        dir = text;
        emit started(text);
    }
}

void MainWindow::on_removeButton_clicked()
{
    if (dir != "" || removing) return;
    labelDupes->setText("doing");
    removing = true;
    std::vector<QString> s;
    QTreeWidgetItemIterator it(ui->treeWidget, QTreeWidgetItemIterator::NoChildren);
    std::vector<QTreeWidgetItem *> d;
    while (*it) {
        if ((*it)->checkState(2) == Qt::Checked) {
            s.push_back((*it)->text(0));
            qDebug() << (*it)->text(0) << '\n';
            d.push_back(*it);
        }
        ++it;
    }
    for (auto c: d) {
        int cnt = c->parent()->text(1).toInt() - 1;
        c->parent()->setText(1, QString::number(cnt));
        if (cnt == 0) {
            delete c->parent();
        } else {
            delete c;
        }
    }
    emit remove(s);
}

void MainWindow::on_cancelButton_clicked()
{
    qDebug() << "wanted to finish\n";
    emit cancel();
}

void MainWindow::on_select_directory_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->lineEdit->setText(dir);
}

void MainWindow::on_expand_clicked()
{
    ui->treeWidget->expandAll();
}

void MainWindow::on_collapse_clicked()
{
    ui->treeWidget->collapseAll();
}

void MainWindow::on_clear_clicked()
{
    QTreeWidgetItemIterator it(ui->treeWidget, QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        (*it)->setCheckState(2, Qt::Unchecked);
        ++it;
    }
}


void MainWindow::on_select_clicked()
{
    QTreeWidgetItemIterator it(ui->treeWidget, QTreeWidgetItemIterator::NoChildren);
    while (*it) {
        if ((*it)->parent()->indexOfChild(*it) != 0) {
            (*it)->setCheckState(2, Qt::Checked);
        } else {
            (*it)->setCheckState(2, Qt::Unchecked);
        }
        ++it;
    }
}
