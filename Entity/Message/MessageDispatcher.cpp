//
// Created by benek on 10/14/24.
//

#include "MessageDispatcher.h"
#include <iostream>

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
		debug_con << "Message not handled"
				  << "";
#endif
	}
}

void MessageDispatcher::DispatchMessage(double delay, int sender, int receiver, int message, void *ExtraInfo) {
	// get pointers to the sender and receiver
	BaseGameEntity *pSender = EntityManager::Instance()->GetEntityFromID(sender);
	BaseGameEntity *pReceiver = EntityManager::Instance()->GetEntityFromID(receiver);

	// make sure the receiver is valid
	if (pReceiver == nullptr) {
		std::cout << "\nWarning! No Receiver with ID of " << receiver << " found";

		return;
	}

	// create the telegram
	Telegram telegram(0, sender, receiver, message, ExtraInfo);

	// if there is no delay, route telegram immediately
	if (delay <= 0.0f) {
		std::cout << "\nInstant telegram dispatched" << std::endl;
		Discharge(pReceiver, telegram);
	} else {
		// TODO add the delay
		//	  m_DelayedMessages.insert(telegram);
	}
}

//---------------------- DispatchDelayedMessages -------------------------
//
//  This function dispatches any telegrams with a timestamp that has
//  expired. Any dispatched telegrams are removed from the queue
//------------------------------------------------------------------------
void MessageDispatcher::DispatchDelayedMessages() {
	//  double CurrentTime = Clock->GetCurrentTime();

	// now peek at the queue to see if any telegrams need dispatching.
	// remove all telegrams from the front of the queue that have gone
	// past their sell by date
	//  while( !m_DelayedMessages.empty() &&
	//         (m_DelayedMessages.begin()->DispatchTime < CurrentTime) &&
	//         (m_DelayedMessages.begin()->DispatchTime > 0) )
	//  {
	//    //read the telegram from the front of the queue
	//    const Telegram& telegram = *m_DelayedMessages.begin();
	//
	//    //find the recipient
	//    BaseGameEntity* pReceiver = EntityManager::Instance()->GetEntityFromID(telegram.Receiver);
	//
	//    std::cout << "\nQueued telegram ready for dispatch: Sent to " << std::endl;
	//
	//    //send the telegram to the recipient
	//    Discharge(pReceiver, telegram);
	//
	//    //remove it from the queue
	//    m_DelayedMessages.erase(m_DelayedMessages.begin());
	//  }
}
