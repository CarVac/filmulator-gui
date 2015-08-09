#ifndef SQLMODEL_H
#define SQLMODEL_H

#include <QtSql/QSqlQueryModel>
#include <QtSql/QSqlQuery>
#include <QDir>
#include <QFile>

//Look at "Using C++ Models with Qt Quick Views

//class SqlModel : public QSqlQueryModel
class SqlModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit SqlModel(QObject *parent = 0);
    void setQuery(const QSqlQuery &query);
    void generateRoleNames();

    QVariant data(const QModelIndex &item, int role) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QHash<int,QByteArray> roleNames() const;

public slots:
    void updateTable(QString table, int operation);

private:
    QHash<int,QByteArray> m_roleNames;

protected:
    QString tableName;
    virtual QSqlQuery modelQuery()=0;
    virtual void emitChange()=0;
    QSqlQueryModel queryModel;

};

#endif // SQLMODEL_H
