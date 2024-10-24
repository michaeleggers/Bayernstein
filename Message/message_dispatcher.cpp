//
// Created by benek on 10/14/24.
//

#include "message_dispatcher.h"
#include <stdio.h>
#include "../Clock/clock.h"
#include "message_type.h"

// uncomment below to send message info to the debug window
// #define SHOW_MESSAGING_INFO

//--------------------------- Instance ----------------------------------------
//
//   this class is a singleton
//-----------------------------------------------------------------------------
MessageDispatcher *MessageDispatcher::Instance() {
    static MessageDispatcher instance;

    return &instance;
}

//----------------------------- Dispatch ---------------------------------
//
//  see description in header
//------------------------------------------------------------------------
void MessageDispatcher::Discharge(BaseGameEntity *pReceiver, const Telegram &telegram) {
    if (!pReceiver->HandleMessage(telegram)) {
        // telegram could not be handled
#ifdef SHOW_MESSAGING_INFO
        printf("Warning! Message not handled: %s\n", MessageToString(telegram.Message).c_str());
#endif
    }
}

void MessageDispatcher::DispatchMessage(double delay, int sender, int receiver, int message, void *ExtraInfo) {
    // get pointers to the sender and receiver
    BaseGameEntity *pSender = EntityManager::Instance()->GetEntityFromID(sender);
    BaseGameEntity *pReceiver = EntityManager::Instance()->GetEntityFromID(receiver);

    // make sure the receiver is valid
    if (pReceiver == nullptr) {
        printf("\nWarning! No Receiver found with ID: %i\n", receiver);
        return;
    }

    // create the telegram
    Telegram telegram(0, sender, receiver, message, ExtraInfo);

    // if there is no delay, route telegram immediately
    if (delay <= 0.0f) {
        printf("\nInstant telegram dispatched at time: %f by %i for %i. Message is %s\n", Clock->GetTime(),
               pSender->ID(), pReceiver->ID(), MessageToString(message).c_str());
        Discharge(pReceiver, telegram);
    } else {
        double currentTime = Clock->GetTime();

        telegram.DispatchTime = currentTime + delay;

        // and put it in the queue
        m_DelayedMessages.insert(telegram);

        printf("\nDelayed telegram from %i recorded at time %f for %i. Message is %s\n", pSender->ID(),
               Clock->GetTime(), pReceiver->ID(), MessageToString(message).c_str());
        Discharge(pReceiver, telegram);
    }
}

//---------------------- DispatchDelayedMessages -------------------------
//
//  This function dispatches any telegrams with a timestamp that has
//  expired. Any dispatched telegrams are removed from the queue
//------------------------------------------------------------------------
void MessageDispatcher::DispatchDelayedMessages() {
    double currentTime = Clock->GetTime();

    // now peek at the queue to see if any telegrams need dispatching.
    // remove all telegrams from the front of the queue that have gone
    // past their sell by date
    while (!m_DelayedMessages.empty() && (m_DelayedMessages.begin()->DispatchTime < currentTime) &&
           (m_DelayedMessages.begin()->DispatchTime > 0)) {
        // read the telegram from the front of the queue
        const Telegram &telegram = *m_DelayedMessages.begin();
        // find the recipient
        BaseGameEntity *pReceiver = EntityManager::Instance()->GetEntityFromID(telegram.Receiver);

        printf("\nQueued telegram ready for dispatch: Sent to %i\n", pReceiver->ID());

        // send the telegram to the recipient
        Discharge(pReceiver, telegram);

        // remove it from the queue
        m_DelayedMessages.erase(m_DelayedMessages.begin());
    }
}
