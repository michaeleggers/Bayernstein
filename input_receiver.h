#ifndef _INPUT_RECEIVER_H_
#define _INPUT_RECEIVER_H_

/* Interface Class. Implement this if you want to be
*  able to delegate input from HIDs to a class.
*/
class IInputReceiver {

public:
    IInputReceiver()  = default;
    ~IInputReceiver() = default;

    virtual void HandleInput() = 0;

};


#endif


