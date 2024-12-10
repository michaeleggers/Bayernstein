//
// Created by benek on 10/15/24.
//

#ifndef TELEGRAM_H
#define TELEGRAM_H

#include <math.h>
#include <stdio.h>

struct Telegram {
    // messages can be dispatched immediately or delayed for a specified amount
    // of time. If a delay is necessary this field is stamped with the time
    // the message should be dispatched.
    double DispatchTime;

    // the entity that sent this telegram
    int Sender;

    // the entity that is to receive this telegram
    int Receiver;

    // the message itself. These are all enumerated in the file
    //"MessageTypes.h"
    int Message;

    // any additional information that may accompany the message
    void* ExtraInfo;

    Telegram()
        : DispatchTime(-1),
          Sender(-1),
          Receiver(-1),
          Message(-1),
          ExtraInfo(nullptr) {}

    Telegram(const double time, const int sender, const int receiver, const int msg, void* info = nullptr)
        : DispatchTime(time),
          Sender(sender),
          Receiver(receiver),
          Message(msg),
          ExtraInfo(info) {}

    [[nodiscard]] char c_str() const {
        char buffer[ 100 ];
        sprintf(buffer,
                "telegram = time: %f, Sender: %i, Receiver %i, Message: %i",
                DispatchTime,
                Sender,
                Receiver,
                Message);
        return *buffer;
    }
};

// these telegrams will be stored in a priority queue. Therefore the >
// operator needs to be overloaded so that the PQ can sort the telegrams
// by time priority. Note how the times must be smaller than
// SmallestDelay apart before two Telegrams are considered to be the same.
// This is to prevent flooding an agent with identical messages.
const double SmallestDelayInMS = 250;

inline bool operator==(const Telegram& t1, const Telegram& t2) {
    return (fabs(t1.DispatchTime - t2.DispatchTime) < SmallestDelayInMS) && (t1.Sender == t2.Sender)
           && (t1.Receiver == t2.Receiver) && (t1.Message == t2.Message);
}

inline bool operator<(const Telegram& t1, const Telegram& t2) {
    if ( t1 == t2 ) {
        return false;
    }

    else {
        return (t1.DispatchTime < t2.DispatchTime);
    }
}

// handy helper function for dereferencing the ExtraInfo field of the Telegram
// to the required type.
template <class T> inline T DereferenceToType(void* p) {
    return *(T*)(p);
}

#endif // TELEGRAM_H
