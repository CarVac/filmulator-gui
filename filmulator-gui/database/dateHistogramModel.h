/* This class is designed to behave as much like basicSqlModel as possible, but it's
 *  specific to the sparse data of the DateHistogram.
 * It takes the nonzero dates in and inserts them into a full table.
 */
#ifndef DATEHISTOGRAMMODEL_H
#define DATEHISTOGRAMMODEL_H

//#include <QAbstractTableModel>
//#include <QtSql/QSqlQuery>
#include "basicSqlModel.h"

class DateHistogramModel : public BasicSqlModel
{
    Q_OBJECT

public:
    explicit DateHistogramModel(QObject *parent = 0);

    //This'll have to copy everything into matrices of things
    void setQuery(const QSqlQuery &query, const int timezone);

    //This'll access just the matrix of data
    QVariant data(const QModelIndex &index, int role) const;

    //This will return a count of the total number of rows,
    // not just the number of rows returned by the query.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void signalChange() {emit dateHistoModelChanged();}

private:
    int m_rowCount;

signals:
    void dateHistoModelChanged();

protected:
    void emitChange() {emit dateHistoModelChanged();}
    double m_today;
    double m_firstDay;
    std::vector<double> m_dataVector;
};

#endif // DATEHISTOGRAMMODEL_H

