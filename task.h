#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

class Task: public QObject
{
Q_OBJECT
public:
    Task();
public slots:
    void doWork(QString);
    void cancel();
    void remove(std::vector<QString>);
signals:
    void send(std::vector< std::vector<QString> >);
    void finished(QString);
    void scan_started();
private:
    std::vector< std::vector<QString> > equals;
    bool is_canceled = false;
};

#endif // TASK_H
