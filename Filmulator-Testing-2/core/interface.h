#ifndef INTERFACE_H
#define INTERFACE_H

class Interface
{
public:
    virtual void updateProgress(float){};
    virtual bool checkAbort(){return false;}
    virtual unsigned short lookup(unsigned short in){return in;}
};

#endif // INTERFACE_H
