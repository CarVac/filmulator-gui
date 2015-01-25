#ifndef SIGNALSWITCHBOARD_H
#define SIGNALSWITCHBOARD_H

#include <QObject>


//SignalSwitchboard provides a way for all changers of the database to notify all
// viewers of the database of changes without needing such a complete network of
// signals. Every time you change a database, have it send a signal (connected to this
// object) that includes which table got changed, and what sort of change was performed.

class SignalSwitchboard : public QObject
{
    Q_OBJECT
public:
    SignalSwitchboard();

public slots:
    void updateTableIn(QString table, int operation);

signals:
    void updateTableOut(QString table, int operation);
};

#endif // SIGNALSWITCHBOARD_H
