#include "filmimageprovider.h"

void FilmImageProvider::updateHistograms()
{
    for(int i = 0; i < 128; i++)
    {
        lumaHistogram[i] = 0;
        rHistogram[i] = 0;
        gHistogram[i] = 0;
        bHistogram[i] = 0;
    }
    for(int i = 0; i < contrast_image.nr(); i = i + 5)
        for(int j = 0; j < contrast_image.nc(); j = j + 15)
        {
            unsigned short luma = 0.2126*contrast_image(i,j)+0.7151*contrast_image(i,j+1)+0.0722*contrast_image(i,j+2);
            lumaHistogram[luma/512]++;
            rHistogram[contrast_image(i,j)/512]++;
            gHistogram[contrast_image(i,j+1)/512]++;
            bHistogram[contrast_image(i,j+2)/512]++;
        }
    maxBinLuma = 0;
    maxBinR = 0;
    maxBinG = 0;
    maxBinB = 0;
    for(int i = 1; i < 127; i++)
    {
        maxBinLuma = (lumaHistogram[i] > maxBinLuma) ? lumaHistogram[i] : maxBinLuma;
        maxBinR = (rHistogram[i] > maxBinR) ? rHistogram[i] : maxBinR;
        maxBinG = (gHistogram[i] > maxBinG) ? gHistogram[i] : maxBinG;
        maxBinB = (bHistogram[i] > maxBinB) ? bHistogram[i] : maxBinB;

    }
    hist++;
    emit histogramsUpdated();
}
