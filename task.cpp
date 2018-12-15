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
#include <QCryptographicHash>
#include <QByteArray>

namespace std
{
    template <>
    struct hash<QByteArray>
    {
        size_t operator()(const QByteArray &x) const
        {
            size_t seed = 0;
            for (auto c : x)
                seed ^= c;
            return seed;
        }
    };
} // namespace std

Task::Task() {}

void Task::cancel() {
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    is_canceled = true;
}

void Task::doWork(QString dir) {
    qDebug() << QString(__func__) << " from work thread: " << QThread::currentThreadId();
    is_canceled = false;
    equals.clear();
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

    /*for (auto& u: t) {

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
    emit send(equals);*/

    for (auto& u: t) {
        if (is_canceled) { emit scan_finished(); return; }
        auto& v = u.second;
        if (v.size() > 1) {
            std::unordered_map< QByteArray, std::vector<QString> > th;
            for (auto& str: v) {
                if (is_canceled) { emit scan_finished(); return; }
                QCryptographicHash hash(QCryptographicHash::Sha256);
                std::ifstream in(str.toStdString().c_str());
                const int N = 1024;
                char bf[N];
                in.read(bf, N);
                hash.addData(bf, in.gcount());
                th[hash.result()].push_back(str);
            }

            for (auto& x: th) {
                if (is_canceled) { emit scan_finished(); return; }
                auto& y = x.second;
                if (y.size() > 1) {
                    std::vector<QByteArray> h(y.size());
                    for (size_t j = 0; j < y.size(); j++) {
                        QCryptographicHash hash(QCryptographicHash::Sha256);
                        std::ifstream in(y[j].toStdString().c_str());
                        const int N = 1024 * 32;
                        char bf[N];
                        while (!in.eof()) {
                            if (is_canceled) { emit scan_finished(); return; }
                            in.read(bf, N);
                            hash.addData(bf, in.gcount());
                        }
                        h[j] = hash.result();
                    }

                    std::vector< std::vector<size_t> > tmp(1, {0});
                    for (size_t j = 1; j < h.size(); j++) {
                        size_t k;
                        for (k = 0; k < tmp.size(); k++) {
                            if (is_canceled) { emit scan_finished(); return; }
                            if (h[j] == h[tmp[k][0]]) {
                                tmp[k].push_back(j);
                                break;
                            }
                        }
                        if (k == tmp.size()) {
                            tmp.push_back({j});
                        }
                    }

                    for (auto& z: tmp) {
                        if (z.size() > 1) {
                            equals.push_back(std::vector<QString> (z.size()));
                            for (size_t k = 0; k < z.size(); k++) {
                                if (is_canceled) { emit scan_finished(); return; }
                                equals.back()[k] = y[z[k]];
                            }
                        }
                    }
                }
            }
            if (equals.size() >= 20) {
                emit send(equals);
                equals.clear();
            }
        }
    }
    qDebug() << "finished\n";
    emit send(equals);
    equals.clear();
    emit scan_finished();
}
