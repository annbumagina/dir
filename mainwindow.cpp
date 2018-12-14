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

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pushButton->setShortcut(QKeySequence(Qt::Key_Enter));

    labelDupes = new QLabel(ui->statusBar);
    labelDupes->setAlignment((Qt::AlignRight));
    labelDupes->setMinimumSize(labelDupes->sizeHint());

    thread = new QThread;
    t = new Task();
    t->moveToThread(thread);
    connect(t, SIGNAL(send(std::vector< std::vector<QString> >)), this, SLOT(update(std::vector< std::vector<QString> >)));
    connect(this, SIGNAL(started(QString)), t, SLOT(doWork(QString)));
    connect(t, SIGNAL(scan_started()), this, SLOT(scan_started()));
    connect(t, SIGNAL(scan_finished()), this, SLOT(scan_finished()));
    connect(this, SIGNAL(cancel()), t, SLOT(cancel()), Qt::DirectConnection);
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

void MainWindow::scan_finished() {
    labelDupes->setText("finished");
}

void MainWindow::scan_started() {
    labelDupes->setText("doing");
    ui->treeWidget->clear();
}

void MainWindow::on_pushButton_clicked()
{
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    QString text = ui->lineEdit->text();

    thread->start();
    emit started(text);
}

void MainWindow::on_removeButton_clicked()
{

}

void MainWindow::on_cancelButton_clicked()
{
    qDebug() << "wanted to finish\n";
    emit cancel();
}
