#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "sqlModel.h"
#include <QString>

class QueueModel : public SqlModel
{
    Q_OBJECT
public:
    explicit QueueModel(QObject *parent = 0);
    Q_INVOKABLE void setQueueQuery();
    Q_INVOKABLE void clearQueue();

public slots:
    Q_INVOKABLE void deQueue(QString searchID);
    Q_INVOKABLE void enQueue(QString searchID);

protected:
    int index;

    void resetIndex();
    QSqlQuery modelQuery();
    void emitChange() {emit queueChanged();}

signals:
    void queueChanged();
};

#endif // QUEUEMODEL_H
