#include "filmimageprovider.h"
#include <pwd.h>
#include <unistd.h>

FilmImageProvider::FilmImageProvider(QQuickImageProvider::ImageType type) :
    QQuickImageProvider(type, QQuickImageProvider::ForceAsynchronousImageLoading)
{
}

FilmImageProvider::FilmImageProvider() :
    QQuickImageProvider(QQuickImageProvider::Image,
                        QQuickImageProvider::ForceAsynchronousImageLoading)
{
}

FilmImageProvider::~FilmImageProvider()
{
}

QImage FilmImageProvider::requestImage(const QString &id,
                                       QSize *size, const QSize &requestedSize)
{
    //Get home directory
    int myuid = getuid();
    passwd *mypasswd = getpwuid(myuid);
    std::string input_configuration = std::string(mypasswd->pw_dir) +
            "/.filmulator/configuration.txt";

    //Set up various thingys that are normally command line args
    std::vector<std::string> input_filename_list;
    std::vector<float> input_exposure_concentration;
    int highlights = 0;
    bool set_whitepoint = false;
    float whitepoint = 5;
    bool tiff = false;
    bool jpeg_in = false;
    bool tonecurve_out = false;
    input_filename_list.push_back(id.toStdString());
    input_exposure_concentration.push_back(0);

    //Set up things to be read in from the configuration file
    float initial_developer_concentration;
    float reservoir_size;
    float developer_thickness;
    float crystals_per_pixel;
    float initial_crystal_radius;
    float initial_silver_salt_density;
    float developer_consumption_const;
    float crystal_growth_const;
    float silver_salt_consumption_const;
    int total_development_time;
    int agitate_count;
    float development_resolution;
    float film_area;
    float sigma_const;
    float layer_mix_const;
    float layer_time_divisor;
    float std_cutoff;
    int rolloff_boundary;

    initialize(input_configuration,
               initial_developer_concentration,
               reservoir_size,
               developer_thickness,
               crystals_per_pixel,
               initial_crystal_radius,
               initial_silver_salt_density,
               developer_consumption_const,
               crystal_growth_const,
               silver_salt_consumption_const,
               total_development_time,
               agitate_count,
               development_resolution,
               film_area,
               sigma_const,
               layer_mix_const,
               layer_time_divisor,
               std_cutoff,
               rolloff_boundary);

    //Load the image and demosaic it.
    matrix<float> input_image;
    Exiv2::ExifData exifData;
    if(imload(input_filename_list, input_exposure_concentration,
              input_image, tiff, jpeg_in, exifData, highlights))
    {
        qDebug("Error loading images");
        return QImage(0,0,QImage::Format_ARGB32);
    }
    exifData["Exif.Image.ProcessingSoftware"] = "Filmulator";

    //Here we do the film simulation on the image...
    matrix<float> output_density = filmulate(input_image,
                                             initial_developer_concentration,
                                             reservoir_size,
                                             developer_thickness,
                                             crystals_per_pixel,
                                             initial_crystal_radius,
                                             initial_silver_salt_density,
                                             developer_consumption_const,
                                             crystal_growth_const,
                                             silver_salt_consumption_const,
                                             total_development_time,
                                             agitate_count,
                                             development_resolution,
                                             film_area,
                                             sigma_const,
                                             layer_mix_const,
                                             layer_time_divisor,
                                             rolloff_boundary);

    //Postprocessing: normalize and apply tone curve
    int nrows = output_density.nr();
    int ncols = output_density.nc()/3;
    matrix<int> output_r(nrows,ncols);
    matrix<int> output_g(nrows,ncols);
    matrix<int> output_b(nrows,ncols);

    postprocess(output_density,set_whitepoint, whitepoint, tonecurve_out,
                std_cutoff, output_r, output_g, output_b);

    //Normally, here we'd output the file. Instead, we write it to the QImage.
    QImage output = QImage(ncols,nrows,QImage::Format_ARGB32);
    for(int i = 0; i < nrows; i++)
    {
        QRgb *line = (QRgb *)output.scanLine(i);
        for(int j = 0; j < ncols; j++)
        {
            *line = QColor(output_r(i,j),output_g(i,j),output_b(i,j)).rgb();
            line++;
        }
    }
    return output;
}
