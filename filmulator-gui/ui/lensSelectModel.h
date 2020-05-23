#ifndef LENSSELECTMODEL_H
#define LENSSELECTMODEL_H

#include <QAbstractTableModel>
#include <lensfun/lensfun.h>

class LensSelectModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit LensSelectModel(QObject *parent = 0);
    virtual ~LensSelectModel() {};

    QVariant data(const QModelIndex &index, int role) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QHash<int,QByteArray> roleNames() const;

    Q_INVOKABLE void update(QString camera, QString lens);

private:
    QHash<int,QByteArray> m_roleNames;
    int m_rowCount;

    lfDatabase *ldb = new lfDatabase;
    std::vector<QString> makerList;
    std::vector<QString> modelList;
    std::vector<int> scoreList;

};

#endif // LENSSELECTMODEL_H
