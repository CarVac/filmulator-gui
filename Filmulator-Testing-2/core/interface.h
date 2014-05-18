#ifndef INTERFACE_H
#define INTERFACE_H

enum Valid {none, load, demosaic, prefilmulation, filmulation, whiteblack, colorcurve, filmlikecurve};

class Interface
{
public:
    virtual void updateFilmProgress(float) = 0;
    virtual bool checkAbort() = 0;
    virtual unsigned short lookup(unsigned short in) = 0;
    virtual bool isGUI() = 0;
    virtual void setValid( Valid ) = 0;
};

#endif // INTERFACE_H
