#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "sqlModel.h"
#include <QString>
#include <QQueue>

struct QueueOrder {
    QString searchID;
    int visualIndex;
};

class QueueModel : public SqlModel
{
    Q_OBJECT
public:
    explicit QueueModel(QObject *parent = 0);
    Q_INVOKABLE void setQueueQuery();
    Q_INVOKABLE void clearQueue();
    Q_INVOKABLE void move(const QString searchID, const int destIndex);

public slots:
    Q_INVOKABLE void deQueue(const QString searchID);
    Q_INVOKABLE void enQueue(const QString searchID);

protected:
    int maxIndex;

    void resetIndex();
    QSqlQuery modelQuery();
    void emitChange() {emit queueChanged();}

signals:
    void queueChanged();
};

#endif // QUEUEMODEL_H
