#include "filmimageprovider.h"
#include <pwd.h>
#include <unistd.h>
#define TIMEOUT 0.1

FilmImageProvider::FilmImageProvider(QQuickImageProvider::ImageType type) :
    QObject(0),
    QQuickImageProvider(type, QQuickImageProvider::ForceAsynchronousImageLoading)
{
}

FilmImageProvider::FilmImageProvider() :
    QObject(0),
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading)
{
    input_image_valid = false;
    filmulated_image_valid = false;
}

FilmImageProvider::~FilmImageProvider()
{
}

void FilmImageProvider::invalidateInputImage()
{
    QMutexLocker locker(&mutex);
    input_image_valid = false;
    if(time_diff(request_start_time) > TIMEOUT)
        abort = true;
}

void FilmImageProvider::invalidateFilmulation()
{
    QMutexLocker locker(&mutex);
    filmulated_image_valid = false;
    if(time_diff(request_start_time) > TIMEOUT)
        abort = true;
}

QImage FilmImageProvider::requestImage(const QString &id,
                                       QSize *size, const QSize &requestedSize)
{
    mutex.lock();
        gettimeofday(&request_start_time,NULL);
        abort = false;
    mutex.unlock();

    QString tempID = id;
    tempID.remove(tempID.length()-1,1);
    //Get home directory
    int myuid = getuid();
    passwd *mypasswd = getpwuid(myuid);
    std::string input_configuration = std::string(mypasswd->pw_dir) +
            "/.filmulator/configuration.txt";

    //Set up various thingys that are normally command line args
    std::vector<std::string> input_filename_list;
    std::vector<float> input_exposure_compensation;
    int highlights = 0;
    bool set_whitepoint = true;
    bool tiff = false;
    bool jpeg_in = false;
    bool tonecurve_out = false;
    input_filename_list.push_back(tempID.toStdString());
    cout << "Exposure compensation: " << exposurecomp << endl;
    input_exposure_compensation.push_back(exposurecomp);

    //Read in from the configuration file
    initialize(input_configuration, filmParams);
    float std_cutoff = filmParams.std_cutoff;

    if(abort)
        return emptyImage();

    //Load the image and demosaic it.
    mutex.lock();
        bool input_image_valid_copy = input_image_valid;
        input_image_valid = ~input_image_valid;
    mutex.unlock();
    Exiv2::ExifData exifData;
    matrix<float> input_image;
    if (!input_image_valid_copy)
    {
        if(imload(input_filename_list, input_exposure_compensation,
                  input_image, tiff, jpeg_in, exifData, highlights))
        {
            qDebug("Error loading images");
            return QImage(0,0,QImage::Format_ARGB32);
        }
        input_image_cache = input_image;
    }
    else
    {
        qDebug("Using cached demosaic");
        input_image = input_image_cache;
    }

    if(abort)
        return emptyImage();

    mutex.lock();
        bool filmulated_image_valid_copy = filmulated_image_valid;
        filmulated_image_valid = ~filmulated_image_valid;
    mutex.unlock();
    matrix<float> output_density;
    if (!filmulated_image_valid_copy)
    {
        input_image *= pow(2, input_exposure_compensation[0]);

        //Here we do the film simulation on the image...
        if(filmulate(input_image, output_density, abort, filmParams))
            return emptyImage();

        filmulated_image_cache = output_density;
    }
    else
    {
        qDebug("Using cached filmulation");
        output_density = filmulated_image_cache;
    }

    if(abort)
        return emptyImage();

    //Postprocessing: normalize and apply tone curve
    int nrows = output_density.nr();
    int ncols = output_density.nc()/3;
    matrix<int> output_r(nrows,ncols);
    matrix<int> output_g(nrows,ncols);
    matrix<int> output_b(nrows,ncols);

    postprocess(output_density,set_whitepoint, whitepoint, tonecurve_out,
                std_cutoff, output_r, output_g, output_b);

    if(abort)
        return emptyImage();

    //Normally, here we'd output the file. Instead, we write it to the QImage.
    QImage output = QImage(ncols,nrows,QImage::Format_ARGB32);
    for(int i = 0; i < nrows; i++)
    {
        QRgb *line = (QRgb *)output.scanLine(i);
        for(int j = 0; j < ncols; j++)
        {
            *line = QColor(min(255,output_r(i,j)),min(255,output_g(i,j)),min(255,output_b(i,j))).rgb();
            line++;
        }
    }

    tout << "Request time: "
            << time_diff(request_start_time) << " seconds" << endl;
    return output;
}

void FilmImageProvider::setExposureComp(float exposure)
{
    exposurecomp = exposure;
}

void FilmImageProvider::setWhitepoint(float whitepointIn)
{
    whitepoint = whitepointIn;
}

QImage FilmImageProvider::emptyImage()
{
    QImage output;
    return output;
}
