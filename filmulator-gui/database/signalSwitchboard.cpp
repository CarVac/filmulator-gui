#include "signalSwitchboard.h"

SignalSwitchboard::SignalSwitchboard() : QObject(0)
{
}

void SignalSwitchboard::updateTableIn(QString table,
                                     int operation)
{
    emit updateTableOut(table, operation);
}
