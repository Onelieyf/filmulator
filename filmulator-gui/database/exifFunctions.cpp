#include "exifFunctions.h"
#include <iostream>

using std::cout;
using std::endl;
using std::max;
using std::min;

QDateTime exifUtcTime(Exiv2::ExifData exifData, const int cameraTZ)
{
    QString exifDateTime = QString::fromStdString(exifData["Exif.Image.DateTime"].toString());

    QDateTime cameraDateTime = QDateTime::fromString(exifDateTime, "yyyy:MM:dd hh:mm:ss");

    cameraDateTime.setUtcOffset(cameraTZ);

    return cameraDateTime;
}

QString exifLocalDateString(Exiv2::ExifData exifData,
                             const int cameraTZ,
                             const int importTZ,
                             const QString dirConfig)
{
    QDateTime captureLocalDateTime =
            QDateTime::fromTime_t(exifUtcTime(exifData, cameraTZ).toTime_t());

    captureLocalDateTime.setUtcOffset(importTZ);

    return captureLocalDateTime.toString(dirConfig);
}

int exifDefaultRotation(Exiv2::ExifData exifData)
{
    int exifOrientation;
    try
    {
        exifOrientation = (int) exifData["Exif.Image.Orientation"].value().toLong();
    }
    catch (...)
    {
        exifOrientation = 0;
    }
    switch(exifOrientation)
    {
    case 3://upside down
    {
        return 2;
    }
    case 6://right side down
    {
        return 3;
    }
    case 8://left side down
    {
        return 1;
    }
    default:
    {
        return 0;
    }
    }
}

QString exifMake(Exiv2::ExifData exifData)
{
    return QString::fromStdString(exifData["Exif.Image.Make"].toString());
}

QString exifModel(Exiv2::ExifData exifData)
{
    return QString::fromStdString(exifData["Exif.Image.Model"].toString());
}

float exifIso(Exiv2::ExifData exifData)
{
    return exifData ["Exif.Photo.ISOSpeedRatings"].toFloat();
}

QString exifTv(Exiv2::ExifData exifData)
{
    return QString::fromStdString(exifData["Exif.Photo.ExposureTime"].toString());
}

float exifAv(Exiv2::ExifData exifData)
{
    return exifData["Exif.Photo.FNumber"].toFloat();
}

float exifFl(Exiv2::ExifData exifData)
{
    return exifData["Exif.Photo.FocalLength"].toFloat();
}

int exifRating(Exiv2::ExifData exifData, Exiv2::XmpData xmpData)
{
    std::string maker = exifData["Exif.Image.Make"].toString();
    if (maker.compare("Canon") == 0)
    {
        return min(5,max(0,(int) xmpData["Xmp.xmp.Rating"].toLong()));
    }
    return 0;
}

float nikonFocalLength(const unsigned int inputFL)
{
    if (inputFL <=   0) {return 4.5;}
    if (inputFL <=  17) {return 8;}
    if (inputFL <=  25) {return 10;}
    if (inputFL <=  26) {return 10.5;}
    if (inputFL <=  28) {return 11;}
    if (inputFL <=  31) {return 12;}
    if (inputFL <=  35) {return 13;}//a coverage guess; there's no electronic 13mm lens to reference against
    if (inputFL <=  36) {return 14;}
    if (inputFL <=  39) {return 15;}
    if (inputFL <=  41) {return 16;}
    if (inputFL <=  43) {return 17;}
    if (inputFL <=  44) {return 18;}
    if (inputFL <=  47) {return 19;}
    if (inputFL <=  48) {return 20;}
    if (inputFL <=  50) {return 21;}
    if (inputFL <=  55) {return 24;}
    if (inputFL <=  56) {return 25;}
    if (inputFL <=  60) {return 28;}
    if (inputFL <=  63) {return 30;}
    if (inputFL <=  68) {return 35;}
    if (inputFL <=  72) {return 40;}
    if (inputFL <=  76) {return 45;}
    if (inputFL <=  80) {return 50;}
    if (inputFL <=  83) {return 55;}
    if (inputFL <=  85) {return 58;}
    if (inputFL <=  86) {return 60;}
    if (inputFL <=  92) {return 70;}
    if (inputFL <=  94) {return 75;}
    if (inputFL <=  97) {return 80;}
    if (inputFL <=  98) {return 85;}
    if (inputFL <= 100) {return 90;}
    if (inputFL <= 104) {return 100;}
    if (inputFL <= 106) {return 105;}
    if (inputFL <= 111) {return 120;}
    if (inputFL <= 112) {return 125;}
    if (inputFL <= 115) {return 135;}
    if (inputFL <= 116) {return 140;}
    if (inputFL <= 118) {return 150;}
    if (inputFL <= 123) {return 170;}
    if (inputFL <= 124) {return 180;}
    if (inputFL <= 128) {return 200;}
    if (inputFL <= 130) {return 210;}
    if (inputFL <= 134) {return 240;}
    if (inputFL <= 136) {return 250;}
    if (inputFL <= 138) {return 270;}
    if (inputFL <= 142) {return 300;}
    if (inputFL <= 151) {return 400;}
    if (inputFL <= 160) {return 500;}
    if (inputFL <= 166) {return 600;}
    if (inputFL <= 176) {return 800;}
    if (inputFL <= 190) {return 1200;}
    if (inputFL <= 202) {return 1700;}
    return 0;
}

QString nikonAperture(const unsigned int inputAperture)
{
    if (inputAperture <=   7) {return "0.95";}//a coverage guess; I don't know what the Z 50/.95 is
    if (inputAperture <=   8) {return "1.2";}
    if (inputAperture <=  12) {return "1.4";}
    if (inputAperture <=  20) {return "1.8";}
    if (inputAperture <=  24) {return "2";}
    if (inputAperture <=  30) {return "2.4";}
    if (inputAperture <=  32) {return "2.5";}
    if (inputAperture <=  35) {return "2.7";}
    if (inputAperture <=  37) {return "2.8";}
    if (inputAperture <=  42) {return "3.3";}
    if (inputAperture <=  44) {return "3.5";}
    if (inputAperture <=  47) {return "3.8";}
    if (inputAperture <=  50) {return "4";}
    if (inputAperture <=  54) {return "4.5";}
    if (inputAperture <=  56) {return "5";}
    if (inputAperture <=  61) {return "5.6";}
    if (inputAperture <=  64) {return "6.3";}
    if (inputAperture <=  67) {return "6.7";}
    if (inputAperture <=  68) {return "7.2";}
    if (inputAperture <=  72) {return "8";}
    if (inputAperture <=  78) {return "9.5";}
    if (inputAperture <=  84) {return "11";}
    if (inputAperture <=  90) {return "13";}
    if (inputAperture <=  94) {return "15";}
    if (inputAperture <=  96) {return "16";}
    if (inputAperture <= 102) {return "19";}
    if (inputAperture <= 108) {return "22";}
    return "";
}
