#include "importWorker.h"
#include "../database/database.hpp"
#include "QThread"
#include "logging.h"
#include <libraw/libraw.h>

ImportWorker::ImportWorker(QObject *parent) : QObject(parent) {}

QString ImportWorker::importFile(const QFileInfo infoIn,
  const int importTZ,
  const int cameraTZ,
  const QString photoDir,
  const QString backupDir,
  const QString dirConfig,
  const QDateTime importStartTime,
  const bool appendHash,
  const bool importInPlace,
  const bool replaceLocation,
  const bool noThumbnail)
{
  // Generate a hash of the raw file.
  QCryptographicHash hash(QCryptographicHash::Md5);
  QFile file(infoIn.absoluteFilePath());
  if (!file.open(QIODevice::ReadOnly)) { FILM_ERROR("File couldn't be opened."); }

  // Check that the raw file is readable by libraw before proceeding
  const std::string abspath = infoIn.absoluteFilePath().toStdString();
  FILM_INFO("importFile absolute file path: {}", abspath);

  std::unique_ptr<LibRaw> libraw = std::unique_ptr<LibRaw>(new LibRaw());

  int libraw_error;
#if (defined(_WIN32) || defined(__WIN32__))
  const QString tempFilename = QString::fromStdString(abspath);
  std::wstring wstr = tempFilename.toStdWString();
  libraw_error = libraw->open_file(wstr.c_str());
#else
  const char *cstrfilename = abspath.c_str();
  libraw_error = libraw->open_file(cstrfilename);
#endif
  if (libraw_error) {
    FILM_ERROR("importFile: libraw could not read input file!");
    FILM_ERROR("libraw error text: {}", libraw_strerror(libraw_error));
    emit doneProcessing(false);
    return "";
  }
#define OPTIONS libraw->imgdata.rawparams.options
  if (libraw->is_floating_point()) {
    FILM_INFO("importFile: floating point raw");
    // tell libraw to not convert to int when unpacking.
    // may not be necessary here but whatever, just in case
    OPTIONS = OPTIONS & ~LIBRAW_RAWOPTIONS_CONVERTFLOAT_TO_INT;
  }
  libraw_error = libraw->unpack();
  if (libraw_error) {
    FILM_ERROR("importFile: libraw could not unpack input file!");
    FILM_ERROR("libraw error text: {}", libraw_strerror(libraw_error));
    emit doneProcessing(false);
    return "";
  }

  // Load data into the hash function.
  while (!file.atEnd()) { hash.addData(file.read(8192)); }
  QString hashString = QString(hash.result().toHex());

  QString filename = infoIn.fileName();
  // Optionally append 7 alphanumeric characters derived from the hash to the filename
  // We don't want it to do this if we're importing in place.
  if (!importInPlace && appendHash) {
    QString subFilename = filename.left(filename.length() - 4);
    QString extension = filename.right(4);
    subFilename.append("_");
    int carry = 0;
    const char a[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QByteArray hashArray = hash.result();

    for (int i = 0; i < 7; i++) {
      // Convert the byte to an integer.
      int value = carry + uint8_t(hashArray.at(i));
      // Carry it so that it affects the next one.
      carry = value / 62;
      int val = value % 62;
      subFilename.append(a[val]);
    }
    subFilename.append(extension);
    filename = subFilename;
  }

  // Set up the main directory to insert the file, and the full file path.
  // This is based on what time it was in the timezone of photo capture.
  QString outputPath = photoDir;
  outputPath.append(exifLocalDateString(abspath, cameraTZ, importTZ, dirConfig));
  QString outputPathName = outputPath;
  outputPathName.append(filename);
  // Create the directory.
  if (!importInPlace) {
    QDir dir(outputPath);
    dir.mkpath(outputPath);
  }

  // Sets up the backup directory.
  QString backupPath = backupDir;
  backupPath.append(exifLocalDateString(abspath, cameraTZ, importTZ, dirConfig));
  QString backupPathName = backupPath;
  backupPathName.append(filename);

  // Create the directory, if the root exists, and we're not importing in place.
  QDir backupRoot(backupDir);
  if (backupRoot.exists() && !importInPlace) {
    QDir backupDirectory(backupPath);
    backupDirectory.mkpath(backupPath);

    // We need to verify that this copy happens successfully
    // And that the file integrity was maintained.
    // I will have it retry up to five times upon failure;
    // In my own experience, this copy has failed
    //  while the main copy that occurs later copies successfully...
    // Is this a caching thing? I dunno.
    bool success = false;
    int attempts = 0;
    if (QFile::exists(backupPathName))// check the integrity of any file that already exists
    {
      QFile backupFile(backupPathName);
      QCryptographicHash backupHash(QCryptographicHash::Md5);
      if (!backupFile.open(QIODevice::ReadOnly)) {
        FILM_ERROR("backup file existed but could not be opened.");
      } else {
        while (!backupFile.atEnd()) { backupHash.addData(backupFile.read(8192)); }
      }
      QString backupHashString = QString(hash.result().toHex());
      if (backupHashString != hashString) {
        FILM_WARN("Backup hash check failed");
        FILM_DEBUG("Original hash: {}", hashString.toStdString());
        FILM_DEBUG("Backup hash:   {}", backupHashString.toStdString());
        success = false;
        backupFile.remove(backupPathName);
      } else {
        FILM_INFO("Backup hash verified");
        success = true;
      }
    }
    while (!success) {
      attempts += 1;
      success = QFile::copy(infoIn.absoluteFilePath(), backupPathName);
      QFile backupFile(backupPathName);
      QCryptographicHash backupHash(QCryptographicHash::Md5);
      if (!backupFile.open(QIODevice::ReadOnly)) {
        FILM_ERROR("backup file could not be opened.");
      } else {
        while (!backupFile.atEnd()) { backupHash.addData(backupFile.read(8192)); }
      }
      QString backupHashString = QString(hash.result().toHex());
      if (backupHashString != hashString) {
        FILM_WARN("Backup attempt number {} hash failed", attempts);
        FILM_DEBUG("Original hash: {}", hashString.toStdString());
        FILM_DEBUG("Backup hash:   {}", backupHashString.toStdString());
        success = false;
        backupFile.remove(backupPathName);
      }
      if (attempts > 6) {
        FILM_ERROR("Giving up on backup.");
        success = true;
      }
    }
  }

  // Check to see if it's already present in the database.
  // Open a new database connection for the thread
  QSqlDatabase db = getDB();
  QSqlQuery query(db);
  query.prepare("SELECT FTfilepath FROM FileTable WHERE (FTfileID = ?);");
  query.bindValue(0, hashString);
  query.exec();
  const bool inDatabaseAlready = query.next();
  QString dbRecordedPath;
  if (inDatabaseAlready) { dbRecordedPath = query.value(0).toString(); }
  db.close();
  // If it's not in the database yet,
  // And we're not updating locations
  //   (if we are updating locations, we don't want it to add new things to the db)
  bool changedST = false;
  QString STsearchID;
  if (!inDatabaseAlready && !replaceLocation) {
    // Record the file location in the database.
    if (!importInPlace) {
      // Copy the file into our main directory.
      // We need to verify that this copy happens successfully
      // And that the file integrity was maintained.
      // I will have it retry up to five times upon failure;
      // In my own experience, the main copy has succeeded
      //  while the earlier backup failed...
      // Is this a caching thing? I dunno.
      bool success = false;
      int attempts = 0;
      while (!success) {
        attempts += 1;
        success = QFile::copy(infoIn.absoluteFilePath(), outputPathName);
        QFile outputFile(outputPathName);
        QCryptographicHash outputHash(QCryptographicHash::Md5);
        if (!outputFile.open(QIODevice::ReadOnly)) {
          FILM_ERROR("output file could not be opened.");
        } else {
          while (!outputFile.atEnd()) { outputHash.addData(outputFile.read(8192)); }
        }
        QString outputHashString = QString(hash.result().toHex());
        if (outputHashString != hashString) {
          FILM_WARN("output attempt number {} hash failed", attempts);
          FILM_DEBUG("Original hash: {}", hashString.toStdString());
          FILM_DEBUG("Output hash:   {}", outputHashString.toStdString());
          success = false;
          outputFile.remove(outputPathName);
        } else {
          // success
          fileInsert(hashString, outputPathName);
        }
        if (attempts > 6) {
          FILM_ERROR("Giving up on output.");
          success = true;
        }
      }
    } else {
      // If it's being imported in place, then we don't copy the file.
      fileInsert(hashString, infoIn.absoluteFilePath());
    }

    // Now create a profile and a search table entry, and a thumbnail.
    STsearchID =
      createNewProfile(hashString, filename, exifUtcTime(abspath, cameraTZ), importStartTime, abspath, noThumbnail);

    // Request that we enqueue the image.
    FILM_INFO("importFile SearchID: {}", STsearchID.toStdString());
    if (QString("") != STsearchID) { emit enqueueThis(STsearchID); }
    // It might be ignored downstream, but that's not our problem here.

    // Tell the views we need updating.
    changedST = true;
  } else if (inDatabaseAlready)// it's already in the database, so just move the file.
  {
    // See if the file is in its old location, and copy if not.
    // DON'T do this if we're updating the location.
    if (!QFile::exists(dbRecordedPath) && !importInPlace && !replaceLocation) {
      // Copy the file into our main directory.
      // We need to verify that this copy happens successfully
      // And that the file integrity was maintained.
      // I will have it retry up to five times upon failure;
      // In my own experience, the main copy has succeeded
      //  while the earlier backup failed...
      // Is this a caching thing? I dunno.
      bool success = false;
      int attempts = 0;
      while (!success) {
        attempts += 1;
        success = QFile::copy(infoIn.absoluteFilePath(), outputPathName);
        QFile outputFile(outputPathName);
        QCryptographicHash outputHash(QCryptographicHash::Md5);
        if (!outputFile.open(QIODevice::ReadOnly)) {
          FILM_ERROR("output file could not be opened.");
        } else {
          while (!outputFile.atEnd()) { outputHash.addData(outputFile.read(8192)); }
        }
        QString outputHashString = QString(hash.result().toHex());
        if (outputHashString != hashString) {
          FILM_WARN("output attempt number {} hash failed", attempts);
          FILM_DEBUG("Original hash: {}", hashString.toStdString());
          FILM_DEBUG("Output hash:   {}", outputHashString.toStdString());
          success = false;
          outputFile.remove(outputPathName);
        } else {
          // success
          fileInsert(hashString, outputPathName);
        }
        if (attempts > 6) {
          FILM_ERROR("Giving up on output.");
          success = true;
        }
      }
    }

    // If we want to update the location of the file.
    if (replaceLocation) {
      fileInsert(hashString, infoIn.absoluteFilePath());
      FILM_INFO("importWorker replace location: {}", infoIn.absoluteFilePath().toStdString());

      STsearchID = hashString.append(QString("%1").arg(1, 4, 10, QLatin1Char('0')));
      FILM_INFO("importWorker replace STsearchID: {}", STsearchID.toStdString());

      if (QString("") != STsearchID) { emit enqueueThis(STsearchID); }
    }
  } else {// it's not in the database but we are hoping to replace the location.
    // We only do this for CLI-based processing.
    if (noThumbnail) {
      fileInsert(hashString, infoIn.absoluteFilePath());
      FILM_INFO("importWorker replace location: {}", infoIn.absoluteFilePath().toStdString());

      // Now create a profile and a search table entry, and a thumbnail.
      STsearchID =
        createNewProfile(hashString, filename, exifUtcTime(abspath, cameraTZ), importStartTime, abspath, noThumbnail);

      // Request that we enqueue the image.
      FILM_INFO("importFile SearchID: {}", STsearchID.toStdString());
      if (QString("") != STsearchID) { emit enqueueThis(STsearchID); }
      // It might be ignored downstream, but that's not our problem here.

      // Tell the views we need updating.
      changedST = true;
    }
  }

  // Tell the ImportModel whether we did anything to the SearchTable
  emit doneProcessing(changedST);
  return STsearchID;
}
