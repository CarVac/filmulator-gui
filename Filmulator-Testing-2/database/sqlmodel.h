#ifndef SQLMODEL_H
#define SQLMODEL_H

#include <QtSql/QSqlTableModel>
#include <QtSql/QSqlError>

class SqlModel : public QSqlTableModel
{
    Q_OBJECT

    void generateRoleNames();

public:
    explicit SqlModel(QObject *parent = 0);


    void setQuery(const QString &query, const QSqlDatabase &db = QSqlDatabase());
    void setTable(const QString &query);
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int,QByteArray> roleNames() const;

    Q_INVOKABLE void test_output() const;
    Q_INVOKABLE void test_addRecord(QString direc, QString filenam, QString exten);
    
signals:

public slots:

private:
    QHash<int,QByteArray> m_roleNames;
    QSqlError error;
};

#endif // SQLMODEL_H
