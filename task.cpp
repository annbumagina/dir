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
#include <stdio.h>

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
        if (is_canceled) { emit finished("canceld"); return; }
        QString file_path = it.next();
        QFile file(file_path);
        auto s = file.size();
        if (file.open(QIODevice::ReadOnly)) {
            t[s].push_back(file_path);
            file.close();
        }
    }

    for (auto& u: t) {
        if (is_canceled) { emit finished("canceld"); return; }
        auto& v = u.second;
        if (v.size() > 1) {
            std::unordered_map< QByteArray, std::vector<QString> > th;
            for (auto& str: v) {
                if (is_canceled) { emit finished("canceld"); return; }
                QCryptographicHash hash(QCryptographicHash::Sha256);
                std::ifstream in(str.toStdString().c_str());
                const int N = 1024;
                char bf[N];
                in.read(bf, N);
                hash.addData(bf, in.gcount());
                th[hash.result()].push_back(str);
            }

            for (auto& x: th) {
                if (is_canceled) { emit finished("canceld"); return; }
                auto& y = x.second;
                if (y.size() > 1) {
                    std::vector<QByteArray> h(y.size());
                    for (size_t j = 0; j < y.size(); j++) {
                        QCryptographicHash hash(QCryptographicHash::Sha256);
                        std::ifstream in(y[j].toStdString().c_str());
                        const int N = 1024 * 32;
                        char bf[N];
                        while (!in.eof()) {
                            if (is_canceled) { emit finished("canceld"); return; }
                            in.read(bf, N);
                            hash.addData(bf, in.gcount());
                        }
                        h[j] = hash.result();
                    }

                    std::vector< std::vector<size_t> > tmp(1, {0});
                    for (size_t j = 1; j < h.size(); j++) {
                        size_t k;
                        for (k = 0; k < tmp.size(); k++) {
                            if (is_canceled) { emit finished("canceld"); return; }
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
                                if (is_canceled) { emit finished("canceld"); return; }
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
    emit finished("finished");
}

void Task::remove(std::vector<QString> v) {
    is_canceled = false;
    for (auto& s: v) {
        if (is_canceled) { emit finished("canceld"); return; }
        std::remove(s.toStdString().c_str());
    }
    emit finished("removed");
}
