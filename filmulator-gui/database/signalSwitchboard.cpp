#include "signalSwitchboard.h"
#include <iostream>

using namespace std;

SignalSwitchboard::SignalSwitchboard() : QObject(0)
{
}

void SignalSwitchboard::updateTableIn(QString table,
                                     int operation)
{
    //cout << "SignalSwitchboard::updateTableIn: table: " << table.toStdString();
    //cout << " operation: " << operation << endl;
    emit updateTableOut(table, operation);
}
