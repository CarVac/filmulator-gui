#ifndef INTERFACE_H
#define INTERFACE_H

class Interface
{
public:
    virtual void updateProgress(float){};
    virtual bool checkAbort(){return false;}
    virtual unsigned short lookup(unsigned short in){return in;}
    virtual bool isGUI(){return false;}
};

#endif // INTERFACE_H
