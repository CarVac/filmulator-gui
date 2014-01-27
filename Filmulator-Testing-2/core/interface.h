#ifndef INTERFACE_H
#define INTERFACE_H

class Interface
{
public:
    virtual void updateProgress(float){};
    virtual bool checkAbort(){return false;}
};

#endif // INTERFACE_H
