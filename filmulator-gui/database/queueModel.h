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
    explicit QueueModel(QObject *parent = nullptr);
    Q_INVOKABLE void setQueueQuery();
    Q_INVOKABLE void clearQueue();
    Q_INVOKABLE void move(const QString searchID, const int destIndex);
    Q_INVOKABLE void markSaved(const QString searchID);

    Q_INVOKABLE void batchEnqueue(const QString searchQuery);
    Q_INVOKABLE void batchForget();

    Q_INVOKABLE QString getNext(const QString searchID);
    Q_INVOKABLE QString getPrev(const QString searchID);

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
    void searchTableChanged();
};

#endif // QUEUEMODEL_H
