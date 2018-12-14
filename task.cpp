#include "task.h"

#include <QDir>
#include <QDirIterator>
#include <QIODevice>
#include <QTextStream>
#include <QFile>
#include <algorithm>
#include <QString>
#include <unordered_map>
#include <cstring>
#include <fstream>
#include <QDebug>
#include <string>
#include <QThread>

Task::Task() {}

void Task::cancel() {
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    is_canceled = true;
}

void Task::doWork(QString dir) {
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();

    emit scan_started();
    std::unordered_map< unsigned long long, std::vector<QString> > t;
    QDirIterator it(dir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {

        if (is_canceled) { emit scan_finished(); return; }

        QString file_path = it.next();
        if (it.fileInfo().isSymLink()) continue;
        QFile file(file_path);
        auto s = file.size();
        t[s].push_back(file_path);
    }

    for (auto& u: t) {

        if (is_canceled) { emit scan_finished(); return; }

        auto& v = u.second;
        //qDebug() << v.size() << '\n';
        if (v.size() > 1) {
            int id = equals.size();
            equals.push_back({v[0]});
            for (size_t j = 1; j < v.size(); j++) {
                size_t k;
                for (k = id; k < equals.size(); k++) {
                    std::ifstream in1(equals[k][0].toStdString().c_str());
                    std::ifstream in2(v[j].toStdString().c_str());
                    //qDebug() << equals[k][0] << ' ' << v[j] << '\n';
                    bool eq = true;
                    const int N = 1024 * 8;
                    char bf1[N], bf2[N];
                    while (!in1.eof() || !in2.eof()) {

                        if (is_canceled) { emit scan_finished(); return; }

                        in1.read(bf1, N);
                        in2.read(bf2, N);
                        if (in1.gcount() != in2.gcount()) {
                            eq = false;
                            break;
                        }
                        //qDebug() << memcmp(bf1, bf2, in1.gcount()) << ' ' << equals[k][0] << ' ' << v[j] << ' ' << in1.gcount() << ' ' << QString(bf1) << ' ' << QString(bf2) << '\n';
                        if (memcmp(bf1, bf2, in1.gcount()) != 0) {
                            eq = false;
                            break;
                        }
                    }

                    if (eq) {
                        equals[k].push_back(v[j]);
                        break;
                    }
                }
                if (k == equals.size()) {
                    equals.push_back({v[j]});
                }
            }
        }
        if (equals.size() == 20) {
            equals.erase(std::remove_if(equals.begin(), equals.end(), [](std::vector<QString> x) { return x.size() == 1; }), equals.end());
            emit send(equals);
            equals.clear();
        }
    }
    qDebug() << "finished\n";
    equals.erase(std::remove_if(equals.begin(), equals.end(), [](std::vector<QString> x) { return x.size() == 1; }), equals.end());
    emit send(equals);

    /*for (auto& u: t) {

            if (is_canceled) { emit scan_finished(); return; }

            auto& v = u.second;
            if (v.size() > 1) {

            }
    }*/
    emit scan_finished();
}
