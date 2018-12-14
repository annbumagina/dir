#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <QString>

class Task: public QObject
{
Q_OBJECT
public:
    Task(QString);
public slots:
    void doWork();
signals:
    void send(std::vector< std::vector<QString> >);
private:
    QString dir;
    std::vector< std::vector<QString> > equals;
};

#endif // TASK_H
