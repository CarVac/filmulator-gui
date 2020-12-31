#ifndef CAMCONST_H
#define CAMCONST_H

#include <QString>

enum camconst_status {
    CAMCONST_DL_OK,
    CAMCONST_DL_INITFAILED,
    CAMCONST_DL_FOPENFAILED,
    CAMCONST_DL_RETRIEVEFAILED,
    CAMCONST_READ_OK,
    CAMCONST_READ_FAILED
};

QString camconst_dir();

camconst_status camconst_download();

camconst_status camconst_read(const QString filePath);


#endif // CAMCONST_H
