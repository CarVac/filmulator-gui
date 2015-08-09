/* This is a minimal implementation of SqlModel
 * It's used when you don't need direct interaction with the gui, and thus don't need
 *  a gigantic list of Q_PROPERTY's.
 */
#ifndef BASICSQLMODEL_H
#define BASICSQLMODEL_H

#include "sqlModel.h"

class BasicSqlModel : public SqlModel
{
    Q_OBJECT

public:
    explicit BasicSqlModel(QObject *parent = 0);
    void setQuery(const QSqlQuery &query);
    void signalChange() {emit basicSqlModelChanged();}
signals:
    void basicSqlModelChanged();
protected:
    QSqlQuery m_modelQuery;
    QSqlQuery modelQuery() {return m_modelQuery;}
    void emitChange() {emit basicSqlModelChanged();}
};

#endif // BASICSQLMODEL_H

