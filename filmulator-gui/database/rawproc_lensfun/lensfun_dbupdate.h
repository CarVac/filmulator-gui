//from https://github.com/butcherg/rawproc
#ifndef LENSFUN_DBUPDATE_H
#define LENSFUN_DBUPDATE_H

#include <string>

enum lf_db_return {
	LENSFUN_DBUPDATE_OK,		//database file retrieve and install was successful
	LENSFUN_DBUPDATE_NOVERSION,	//version requested is not available
	LENSFUN_DBUPDATE_NODATABASE,	//no database installed locally at the path
	LENSFUN_DBUPDATE_OLDVERSION,	//database installed locally at the path is not the latest
	LENSFUN_DBUPDATE_CURRENTVERSION,//database installed locally at the path is already the latest version
	LENSFUN_DBUPDATE_RETRIEVE_OK,	//database file retrieve from server succeeded
	LENSFUN_DBUPDATE_RETRIEVE_INITFAILED,	//libcurl init
	LENSFUN_DBUPDATE_RETRIEVE_FILEOPENFAILED,	//local file
	LENSFUN_DBUPDATE_RETRIEVE_RETRIEVEFAILED	//actual retrieve
};

//checks available database version, state of installed version:
lf_db_return lensfun_dbcheck(int version, std::string dbpath=std::string(), std::string dburl=std::string());

//does the full data base availbility and version checks, retrieves and installs new database:
lf_db_return lensfun_dbupdate(int version, std::string dbpath=std::string(), std::string dburl=std::string());

#endif
