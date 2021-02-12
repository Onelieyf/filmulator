#ifndef FILMIMAGEPROVIDER_H
#define FILMIMAGEPROVIDER_H

#include <QObject>
#include <QColor>
#include <QImage>
#include <QQuickImageProvider>
#include <QMutex>
#include <QMutexLocker>
#include "filmsim.hpp"

enum Valid {none, demosaic, filmulation, whiteblack, colorcurve, filmlikecurve};

class FilmImageProvider : public QObject, public QQuickImageProvider, public Interface
{
    Q_OBJECT
    Q_PROPERTY(float exposureComp READ getExposureComp WRITE setExposureComp NOTIFY exposureCompChanged)
    Q_PROPERTY(float whitepoint READ getWhitepoint WRITE setWhitepoint NOTIFY whitepointChanged)
    Q_PROPERTY(float progress READ getProgress WRITE setProgress NOTIFY progressChanged)
public:
    FilmImageProvider(QQuickImageProvider::ImageType type);
    FilmImageProvider();
    ~FilmImageProvider();
    QImage requestImage(const QString& id, QSize* size, const QSize& requestedSize);

    void setExposureComp(float exposureIn);
    void setWhitepoint(float whitepointIn);
    void setProgress(float progressIn);

    float getExposureComp(){return exposureComp;}
    float getWhitepoint(){return whitepoint;}
    float getProgress(){return progress;}

    bool checkAbort(Valid currStep);
    bool checkAbort(){return checkAbort(filmulation);}

    void updateProgress(float);
    Q_INVOKABLE void invalidateImage();

protected:
    QMutex mutex;

    float exposureComp;
    float progress;
    float whitepoint;
    float blackpoint;

    LUT lutR, lutG, lutB;
    LUT filmLikeLUT;

    filmulateParams filmParams;

    struct timeval request_start_time;

    Valid valid;

    matrix<float> input_image;
    Exiv2::ExifData exifData;
    matrix<float> filmulated_image;
    matrix<unsigned short> contrast_image;
    matrix<unsigned short> color_curve_image;
    matrix<unsigned short> film_curve_image;

    QImage emptyImage();

signals:
    void exposureCompChanged();
    void whitepointChanged();
    void progressChanged();

public slots:

};

#endif // FILMIMAGEPROVIDER_H
