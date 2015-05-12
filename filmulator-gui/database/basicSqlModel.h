#ifndef BASICSQLMODEL_H
#define BASICSQLMODEL_H

#include "sqlModel.h"

class BasicSqlModel : public SqlModel
{
    Q_OBJECT

public:
    explicit BasicSqlModel(QObject *parent = 0);
    void setQuery(const QSqlQuery &query);
private:
    QSqlQuery m_modelQuery;
signals:
    void basicSqlModelChanged();
protected:
    QSqlQuery modelQuery() {return m_modelQuery;}
    void emitChange() {emit basicSqlModelChanged();}
};

#endif // BASICSQLMODEL_H

