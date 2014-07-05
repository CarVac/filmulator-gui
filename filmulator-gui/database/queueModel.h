#ifndef QUEUEMODEL_H
#define QUEUEMODEL_H

#include "sqlModel.h"
#include <QString>

class QueueModel : public SqlModel
{
    Q_OBJECT
public:
    explicit QueueModel( QObject *parent = 0 );
    Q_INVOKABLE void setQueueQuery();
    Q_INVOKABLE void deQueue( int index );
    Q_INVOKABLE void enQueue( QString string );

protected:
    int index;

    void resetIndex();

signals:
    void queueChanged();
};

#endif // QUEUEMODEL_H
