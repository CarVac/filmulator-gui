//from https://github.com/butcherg/rawproc
//but reformatted for consistency
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/param.h>

#include <curl/curl.h>
#include <archive.h>
#include <archive_entry.h>


#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>


#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

#include "lensfun_dbupdate.h"

#if (defined(_WIN32) || defined(__WIN32__))
#define mkdir(A, B) mkdir(A)
#endif


//utility routines:

std::vector<std::string> filelist()
{
    std::vector<std::string> files;
    DIR *dir = NULL;
    struct dirent *drnt = NULL;

    dir=opendir(".");
    if(dir)
    {
        while(drnt = readdir(dir))
        {
            std::string file = std::string(drnt->d_name);
            if (file == "." | file == "..") continue;
            files.push_back(file);
        }
        closedir(dir);
    }
    return files;
}

//https://stackoverflow.com/questions/2203159/is-there-a-c-equivalent-to-getcwd
std::string get_cwd()
{
   char temp[MAXPATHLEN];
   return ( getcwd(temp, sizeof(temp)) ? std::string( temp ) : std::string("") );
}

std::string string_format(const std::string fmt, ...) 
{
    int size = ((int)fmt.size()) * 2 + 50;   // Use a rubric appropriate for your code
    std::string str;
    va_list ap;
    while (1) // Maximum two passes on a POSIX system...
    {
        str.resize(size);
        va_start(ap, fmt);
        int n = vsnprintf((char *)str.data(), size, fmt.c_str(), ap);
        va_end(ap);
        if (n > -1 && n < size) // Everything worked
        {
            str.resize(n);
            return str;
        }
        if (n > -1)  // Needed size returned
        {
            size = n + 1;   // For null char
        } else {
            size *= 2;      // Guess at a larger size (OS specific)
        }
    }
    return str;
}

//from https://stackoverflow.com/questions/8520560/get-a-file-name-from-a-path, Pixelchemist:
std::string base_name(std::string const & path, std::string const & delims = "/\\")
{
    return path.substr(path.find_last_of(delims) + 1);
}

std::string remove_extension(std::string const &filename)
{
    typename std::string::size_type const p(filename.find_last_of('.'));
    return p > 0 && p != std::string::npos ? filename.substr(0, p) : filename;
}

std::vector<std::string> split(std::string s, std::string delim)
{
    std::vector<std::string> v;
    if (s.find(delim) == std::string::npos)
    {
        v.push_back(s);
        return v;
    }
    size_t pos=0;
    size_t start;
    while (pos < s.length())
    {
        start = pos;
        pos = s.find(delim,pos);
        if (pos == std::string::npos)
        {
            v.push_back(s.substr(start,s.length()-start));
            return v;
        }
        v.push_back(s.substr(start, pos-start));
        pos += delim.length();
    }
    return v;
}

std::string removeall(std::string str, char c)
{
    std::string out = str;
    out.erase(std::remove(out.begin(), out.end(), c), out.end());
    return out;
}


//curl routines:

struct MemoryStruct
{
    char *memory;
    size_t size;
};
 
static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = (char *) realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL)
    {
        /* out of memory! */
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

std::string getAsString(std::string url)
{
    std::string out;
    CURL *curl_handle;
    CURLcode res;

    struct MemoryStruct chunk;

    chunk.memory = (char *) malloc(1);  /* will be grown as needed by the realloc above */
    chunk.size = 0;    /* no data at this point */

    curl_global_init(CURL_GLOBAL_ALL);

    curl_handle = curl_easy_init();
    curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
    res = curl_easy_perform(curl_handle);

    if(res != CURLE_OK)
    {
        return out;
    } else {
        out.assign(chunk.memory,chunk.size);
    }

    curl_easy_cleanup(curl_handle);
    free(chunk.memory);
    curl_global_cleanup();

    return out;
}

lf_db_return getAndSaveFile(std::string url)
{
    lf_db_return result = LENSFUN_DBUPDATE_RETRIEVE_OK;
    CURL *curl;
    CURLcode res;
    std::string dbfile = base_name(url);

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

        FILE *f = fopen(dbfile.c_str(), "wb");
        if (f)
        {
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, f);
            res = curl_easy_perform(curl);
            if(res != CURLE_OK)
            {
                result = LENSFUN_DBUPDATE_RETRIEVE_RETRIEVEFAILED;
            }
            fclose(f);
        }
        else result = LENSFUN_DBUPDATE_RETRIEVE_FILEOPENFAILED;
        curl_easy_cleanup(curl);
    }
    else result = LENSFUN_DBUPDATE_RETRIEVE_INITFAILED;

    curl_global_cleanup();
    return result;
}

//libarchive routines:

static int copy_data(struct archive *ar, struct archive *aw)
{
    int r;
    const void *buff;
    size_t size;
#if ARCHIVE_VERSION_NUMBER >= 3000000
    int64_t offset;
#else
    off_t offset;
#endif

    for (;;)
    {
        r = archive_read_data_block(ar, &buff, &size, &offset);
        if (r == ARCHIVE_EOF)
        {
            return (ARCHIVE_OK);
        }
        if (r != ARCHIVE_OK)
        {
            return (r);
        }
        r = archive_write_data_block(aw, buff, size, offset);
        if (r != ARCHIVE_OK)
        {
            return (r);
        }
    }
}

static int extract(const char *filename, int flags)
{
    struct archive *a;
    struct archive *ext;
    struct archive_entry *entry;
    int r;

    a = archive_read_new();
    ext = archive_write_disk_new();
    archive_write_disk_set_options(ext, flags);

    archive_read_support_format_all(a);
    archive_read_support_filter_all(a);

    if (filename != NULL && strcmp(filename, "-") == 0)
    {
        filename = NULL;
    }
    if ((r = archive_read_open_filename(a, filename, 10240)))
    {
        return -1; //fail("archive_read_open_filename()",archive_error_string(a), r);
    }

    for (;;)
    {
        r = archive_read_next_header(a, &entry);
        if (r == ARCHIVE_EOF)
        {
            break;
        }

        if (r != ARCHIVE_OK)
        {
            return -2; //fail("archive_read_next_header()",archive_error_string(a), 1);
        }

        r = archive_write_header(ext, entry);
        copy_data(a, ext);
        r = archive_write_finish_entry(ext);
        if (r != ARCHIVE_OK)
        {
            return -3; //fail("archive_write_finish_entry()", archive_error_string(ext), 1);
        }
    }
    archive_read_close(a);
    archive_read_free(a);

    archive_write_close(ext);
    archive_write_free(ext);

    return 1;
}


//the dbupdate functions:

lf_db_return lensfun_dbcheck(int version, std::string dbpath, std::string dburl)
{
    int result;

    //this is the lensfun databaseversion to be installed/updated
    int dbversion = version;

    //if a path to the database is specified, cd to it; otherwise, stay at the cwd:
    if (!dbpath.empty())
    {
        result = chdir(dbpath.c_str());
    }

    std::string repositoryurl = "http://lensfun.sourceforge.net/db/";
    if (!dburl.empty())
    {
        repositoryurl = dburl;
    }

    //build the dir to store the lensfun database:
    std::string dbdir = string_format("version_%d",dbversion);

    //get versions.json:
    std::string versions = getAsString(string_format("%sversions.json",repositoryurl.c_str()));

    //parse timestamp and version numbers from downloaded versions.json:
    std::string foo = removeall(versions,'[');
    foo = removeall(foo,']');
    std::vector<std::string> fields = split(foo,",");
    int timestamp = atoi(fields[0].c_str());
    bool versionavailable = false;
    for (int i=0; i<int(fields.size()); i++)
    {
        if (atoi(fields[i].c_str()) == dbversion)
        {
            versionavailable = true;
        }
    }

    if (!versionavailable)
    {
        //err(string_format("Version %d not available.",dbversion));
        return LENSFUN_DBUPDATE_NOVERSION;
    }

    //check version_x/timestamp.txt against timestamp, if timestamp is <=, tell user the database is already at the current version
    struct stat b;
    if (stat(dbdir.c_str(), &b) == 0)
    {
        std::ifstream tsfile;
        tsfile.open(string_format("%s/timestamp.txt",dbdir.c_str()));
        std::stringstream buffer;
        buffer << tsfile.rdbuf();
        int ts = atoi(buffer.str().c_str());
        if (ts >= timestamp)
        {
            return LENSFUN_DBUPDATE_CURRENTVERSION;
        }
        else
        {
            return LENSFUN_DBUPDATE_OLDVERSION;
        }
    } else {
        return LENSFUN_DBUPDATE_NODATABASE;
    }
}

lf_db_return lensfun_dbupdate(int version, std::string dbpath, std::string dburl)
{
    int result;

    //this is the lensfun databaseversion to be installed/updated
    int dbversion = version;

    //if a path to the database is specified, cd to it; otherwise, stay at the cwd:
    if (!dbpath.empty())
    {
        result = chdir(dbpath.c_str());
    }

    std::string repositoryurl = "http://lensfun.sourceforge.net/db/";
    if (!dburl.empty())
    {
        repositoryurl = dburl;
    }

    //build the dir to store the lensfun database:
    std::string dbdir = string_format("version_%d",dbversion);

    //get versions.json:
    std::string versions = getAsString(string_format("%sversions.json",repositoryurl.c_str()));

    //parse timestamp and version numbers from downloaded versions.json:
    std::string foo = removeall(versions,'[');
    foo = removeall(foo,']');
    std::vector<std::string> fields = split(foo,",");
    int timestamp = atoi(fields[0].c_str());
    bool versionavailable = false;
    for (int i=0; i<int(fields.size()); i++) if (atoi(fields[i].c_str()) == dbversion) versionavailable = true;

    if (!versionavailable)
    {
        //err(string_format("Version %d not available.",dbversion));
        return LENSFUN_DBUPDATE_NOVERSION;
    }

    //check version_x/timestamp.txt against timestamp, if timestamp is <=, tell user the database is already at the current version
    struct stat b;
    bool isdirectory;
    if (stat(dbdir.c_str(), &b) == 0)
    {
        isdirectory = true;
        std::ifstream tsfile;
        tsfile.open(string_format("%s/timestamp.txt",dbdir.c_str()));
        std::stringstream buffer;
        buffer << tsfile.rdbuf();
        int ts = atoi(buffer.str().c_str());
        if (ts >= timestamp)
        {
            //err(string_format("local timestamp: %d >= server timestamp: %d; local database is the most current version.",ts,timestamp));
            return LENSFUN_DBUPDATE_CURRENTVERSION;
        }
    }
    else isdirectory = false;

    //build the url and file to retrieve and store the database:
    std::string dbfile = string_format("%s.tar.bz2",dbdir.c_str());
    std::string databbaseurl = string_format("%s%s",repositoryurl.c_str(),dbfile.c_str());

    //get the database file to the current working directory:
    lf_db_return retrieveresult = getAndSaveFile(databbaseurl);
    if (retrieveresult != LENSFUN_DBUPDATE_RETRIEVE_OK)
    {
        //err(string_format("Retrive %s failed.",databbaseurl.c_str()));
        return retrieveresult;
    }

    //store the working directory before cd'ing down into version_x/:
    std::string prevdir = get_cwd();

    //if the directory doesn't exist, create and cd into it
    if (!isdirectory)
    {
        mkdir(dbdir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
        result = chdir(dbdir.c_str());
    } else { //if the directory exists, cd into it and delete all the files:
        result = chdir(dbdir.c_str());
        std::vector<std::string> flist = filelist();
        for (int i=0; i<int(flist.size()); i++)
        {
            remove(flist[i].c_str());
        }
    }
	
    //extract the database tar.bz2 into the version_x directory:
    result = extract((std::string("../")+dbfile).c_str(), ARCHIVE_EXTRACT_TIME);

    //ToDo: timestamp.txt
    std::string timestampstring = string_format("%d",timestamp);
    std::ofstream out("timestamp.txt");
    out << timestampstring;
    out.close();

    //cd back to the original cwd, remove the version_x.tar.bz2:
    result = chdir(prevdir.c_str());
    remove(dbfile.c_str());
	
    return LENSFUN_DBUPDATE_OK;
}

