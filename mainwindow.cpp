#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDir>
#include <QDirIterator>
#include <QIODevice>
#include <QTextStream>
#include <QFile>
#include <algorithm>

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




std::vector< std::vector<QString> > equals;

void MainWindow::on_pushButton_clicked()
{
    QString text = ui->lineEdit->text();
    //QDir dir(text);
    equals.clear();
    const int n = 100003;
    const unsigned long long p = 65537;
    std::vector< std::vector<QString> > t(n);
    QDirIterator it(text, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    QString s = "";
    while (it.hasNext()) {
        //s += it.next() + '\n';
        QString file_path = it.next();
        QFile file(file_path);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            s += "cannot open:\n" + file_path + "\n\n";
            continue;
        }
        QTextStream stream(&file);
        QString line;
        unsigned long long hash = 0;
        unsigned long long pp = 1;
        while (!stream.atEnd()) {
            line = stream.readLine();
            for (int i = 0; i < line.size(); i++) {
                hash += line[i].unicode() * pp;
                pp *= p;
            }
        }
        hash %= n;
        t[hash].push_back(file_path);
    }



    for (int i = 0; i < n; i++) {
        if (t[i].size() > 1) {
            int id = equals.size();
            equals.push_back(std::vector<QString>(1, t[i][0]));
            for (size_t j = 1; j < t[i].size(); j++) {
                size_t k;
                for (k = id; k < equals.size(); k++) {
                    QFile file1(equals[k][0]), file2(t[i][j]);
                    if (!file1.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        s += "cannot open:\n" + equals[k][0] + "\n\n";
                        break;
                    }
                    if (!file2.open(QIODevice::ReadOnly | QIODevice::Text)) {
                        s += "cannot open:\n" + t[i][1] + "\n\n";
                        break;
                    }
                    QTextStream stream1(&file1), stream2(&file2);
                    QString line1, line2;
                    bool eq = true;
                    while (!stream1.atEnd() && !stream2.atEnd()) {
                        line1 = stream1.readLine();
                        line2 = stream2.readLine();
                        if (line1 != line2) {
                            eq = false;
                            break;
                        }
                    }
                    if (!stream1.atEnd() || !stream2.atEnd()) {
                        eq = false;
                    }
                    if (eq) {
                        equals[k].push_back(t[i][j]);
                        break;
                    }
                }
                if (k == equals.size()) {
                    equals.push_back(std::vector<QString>(1, t[i][j]));
                }
            }
        }
    }
    for (size_t i = 0; i < equals.size(); i++) {
        if (equals[i].size() > 1) {
            s += "equal:\n";
            for (size_t j = 0; j < equals[i].size(); j++) {
                s += equals[i][j] + '\n';
            }
            s += '\n';
        }
    }
    ui->textEdit->setText(s);
}

void MainWindow::on_removeButton_clicked()
{
    bool success = true;
    QString s = "";
    for (size_t i = 0; i < equals.size(); i++) {
        for (size_t j = 1; j < equals[i].size(); j++) {
            QFile f(equals[i][j]);
            if (!f.remove()) {
                success = false;
                s += equals[i][j] + ' ';
            }
        }
    }
    if (success) {
        s = "successful";
    } else {
        s = "failed to remove: " + s;
    }
    ui->failEdit->setText(s);
}
