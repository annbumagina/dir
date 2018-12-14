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

Task::Task(QString s) : dir(s) {

}

void Task::doWork() {
    emit send({{"hi"}});
    std::unordered_map< unsigned long long, std::vector<QString> > t;
    QDirIterator it(dir, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString file_path = it.next();
        if (it.fileInfo().isSymLink()) continue;
        QFile file(file_path);
        auto s = file.size();
        t[s].push_back(file_path);
    }

    for (auto& u: t) {
        auto& v = u.second;
        if (v.size() > 1) {
            int id = equals.size();
            equals.push_back({v[0]});
            for (size_t j = 1; j < v.size(); j++) {
                size_t k;
                for (k = id; k < equals.size(); k++) {

                    /*QFile file1(equals[k][0]), file2(v[j]);
                    file1.open(QIODevice::ReadOnly | QIODevice::Text);
                    file2.open(QIODevice::ReadOnly | QIODevice::Text);
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
                    }*/

                    std::ifstream in1(equals[k][0].toStdString().c_str());
                    std::ifstream in2(v[j].toStdString().c_str());
                    bool eq = true;
                    const int N = 1024;
                    char bf1[N], bf2[N];
                    while (!in1.eof() || !in2.eof()) {
                        in1.read(bf1, N);
                        in2.read(bf2, N);
                        qDebug() << strcmp(bf1, bf2) << ' ' << equals[k][0] << ' ' << v[j] << ' ' << in1.gcount() << ' ' << QString(bf1) << ' ' << QString(bf2) << '\n';
                        if (strcmp(bf1, bf2) != 0) {
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

    equals.erase(std::remove_if(equals.begin(), equals.end(), [](std::vector<QString> x) { return x.size() == 1; }), equals.end());
    emit send(equals);
    emit send({{"bye"}});
}
