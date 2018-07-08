#ifndef BITMAP_H
#define BITMAP_H

inline void BmpBlit(
    void* destData,
    unsigned destW,
    unsigned destH,
    unsigned destBpp,
    void* srcData, 
    unsigned srcW, 
    unsigned srcH,
    unsigned srcBpp,
    unsigned xOffset,
    unsigned yOffset)
{
    unsigned char* dest = (unsigned char*)destData;
    unsigned char* src = (unsigned char*)srcData;
    for(unsigned y = 0; (y + yOffset) < destH && y < srcH; ++y)
    {
        for(unsigned x = 0; (x + xOffset) < destW && x < srcW; ++x)
        {
            unsigned index = ((y + yOffset) * destW + (x + xOffset)) * destBpp;
            unsigned srcIndex = (y * srcW + x) * srcBpp;
            for(unsigned b = 0; b < destBpp && b < srcBpp; ++b)
            {
                dest[index + b] = src[srcIndex + b];
            }
            if(destBpp == 4 && srcBpp < 4)
                dest[index + 3] = 255;
        }
    }
}

#endif
