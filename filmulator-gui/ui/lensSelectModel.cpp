#include "lensSelectModel.h"
#include <QDir>
#include <QStandardPaths>
#include <iostream>
using std::cout;
using std::endl;

LensSelectModel::LensSelectModel(QObject *parent) : QAbstractTableModel(parent)
{
    //generate role names, which are constant
    m_roleNames[Qt::UserRole + 0 + 1] = "make";
    m_roleNames[Qt::UserRole + 1 + 1] = "model";
    m_roleNames[Qt::UserRole + 2 + 1] = "score";

    //it starts off with no rows
    m_rowCount = 0;

    //and nothing in the rows
    makerList.clear();
    modelList.clear();
    scoreList.clear();

    //initialize lensfun db
    QDir dir = QDir::home();
    QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    dirstr.append("/filmulator/version_2");
    std::string stdstring = dirstr.toStdString();
    ldb->Load(stdstring.c_str());
}

void LensSelectModel::update(QString lensString)
{
    beginResetModel();
    cout << "LensSelectModel::update: " << lensString.toStdString() << endl;
    std::string lensStr = lensString.toStdString();

    //clear all the data
    m_rowCount = 0;
    makerList.clear();
    modelList.clear();
    scoreList.clear();

    if (lensStr.length() > 0)
    {
        const lfLens ** lensList = ldb->FindLenses(NULL, NULL, lensStr.c_str());
        if (lensList)
        {
            int i = 0;
            while (lensList[i])
            {
                makerList.push_back(QString(lensList[i]->Maker));
                modelList.push_back(QString(lensList[i]->Model));
                scoreList.push_back(lensList[i]->Score);
                i++;
                m_rowCount++;
                cout << m_rowCount << endl;
            }
        }
        lf_free(lensList);
    }
    endResetModel();
}

QVariant LensSelectModel::data(const QModelIndex &index, int role) const
{
    QVariant value;
    if (role < Qt::UserRole)
    {
        value = 0;
    } else {
        const int col = role - Qt::UserRole - 1;
        const int row = index.row();
        if (col == 0)
        {
            value = makerList[row];
        } else if (col == 1) {
            value = modelList[row];
        } else {
            value = scoreList[row];
        }
    }
    return value;
}

int LensSelectModel::rowCount(const QModelIndex &) const
{
    return m_rowCount;
}

int LensSelectModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QHash<int,QByteArray> LensSelectModel::roleNames() const
{
    return m_roleNames;
}
