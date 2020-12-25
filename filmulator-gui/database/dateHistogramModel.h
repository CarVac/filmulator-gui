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

    Q_PROPERTY(int dateHistoSize READ getDateHistoSize NOTIFY dateHistoSizeChanged)
public:
    explicit DateHistogramModel(QObject *parent = 0);

    //This'll have to copy everything into matrices of things
    void setQuery(const int timezone,
                  const int minRating,
                  const int maxRating);

    //This'll access just the matrix of data
    QVariant data(const QModelIndex &index, int role) const;

    //This will return a count of the total number of rows,
    // not just the number of rows returned by the query.
    int rowCount(const QModelIndex &parent = QModelIndex()) const;

    void signalChange() {emit dateHistoModelChanged();}

    Q_INVOKABLE int getDateHistoSize(){return m_rowCount;}

private:
    int m_rowCount;

signals:
    void dateHistoModelChanged();
    void dateHistoSizeChanged();

protected:
    void emitChange() {emit dateHistoModelChanged();}
    std::vector<double> m_dataVector;
};

#endif // DATEHISTOGRAMMODEL_H

