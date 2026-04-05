#include "lensSelectModel.h"
#include "../core/logging.h"
#include <QDir>
#include <QFileInfo>
#include <QFileInfoList>
#include <QStandardPaths>
#include <iostream>

LensSelectModel::LensSelectModel(QObject *parent) : QAbstractTableModel(parent)
{
  // generate role names, which are constant
  m_roleNames[Qt::UserRole + 0 + 1] = "make";
  m_roleNames[Qt::UserRole + 1 + 1] = "model";
  m_roleNames[Qt::UserRole + 2 + 1] = "score";

  // it starts off with no rows
  m_rowCount = 0;

  // and nothing in the rows
  makerList.clear();
  modelList.clear();
  scoreList.clear();

  // initialize lensfun db
  QString dirstr = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
  dirstr.append("/filmulator/version_2/");
  QDir dir(dirstr);
  QStringList filters;
  filters << "*.xml";
  QFileInfoList fileList = dir.entryInfoList(filters, static_cast<QDir::Filters>(QDir::Files | QDir::NoDotAndDotDot));

  ldb = lf_db_new();
  if (!ldb) { FILM_ERROR("Failed to create database!"); }

  for (const QFileInfo &fileInfo : fileList) {
    const QString filename = fileInfo.absoluteFilePath();
    const std::string stdstring = filename.toStdString();
    ldb->Load(stdstring.c_str());
  }
}

LensSelectModel::~LensSelectModel()
{
  if (ldb != NULL) { lf_db_destroy(ldb); }
}

// If the lensString begins with a backslash, we search all cameras
// This lets you deal with adapted lenses.
void LensSelectModel::update(QString cameraString, QString lensString)
{
  beginResetModel();
  std::string camStr = cameraString.toStdString();

  QString tempLensString = lensString;
  bool searchAllMounts = false;
  if (!lensString.isEmpty()) {
    if (lensString.front() == '\\') {
      tempLensString.remove(0, 1);
      searchAllMounts = true;
    }
  }
  std::string lensStr = tempLensString.toStdString();

  // clear all the data
  m_rowCount = 0;
  makerList.clear();
  modelList.clear();
  scoreList.clear();

  const lfCamera *camera = NULL;
  const lfCamera **cameraList = ldb->FindCameras(NULL, camStr.c_str());
  if (cameraList && !searchAllMounts) {
    camera = cameraList[0];
  } else if (!searchAllMounts) {
    FILM_DEBUG("LensSelectModel lensfun no camera; camStr: {}", camStr);
  }
  lf_free(cameraList);

  if (lensStr.length() > 0) {
    const lfLens **lensList = ldb->FindLenses(camera, NULL, lensStr.c_str());
    if (lensList) {
      int i = 0;
      while (lensList[i]) {
        makerList.push_back(QString(lensList[i]->Maker));
        QString lensModel = lensList[i]->Model;
        if (searchAllMounts) { lensModel = lensModel.prepend("\\"); }
        modelList.push_back(lensModel);
        scoreList.push_back(lensList[i]->Score);
        i++;
        m_rowCount++;
      }
    }
    lf_free(lensList);
  }
  endResetModel();
}

QVariant LensSelectModel::data(const QModelIndex &index, int role) const
{
  QVariant value;
  if (role < Qt::UserRole) {
    value = 0;
  } else {
    const int col = role - Qt::UserRole - 1;
    const int row = index.row();
    if (col == 0) {
      value = makerList[row];
    } else if (col == 1) {
      value = modelList[row];
    } else {
      value = scoreList[row];
    }
  }
  return value;
}

int LensSelectModel::rowCount(const QModelIndex &) const { return m_rowCount; }

int LensSelectModel::columnCount(const QModelIndex &) const { return 3; }

QHash<int, QByteArray> LensSelectModel::roleNames() const { return m_roleNames; }
