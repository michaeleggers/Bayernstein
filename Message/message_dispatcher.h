//
// Created by benek on 10/14/24.
//

#ifndef MESSAGEDISPATCHER_H
#define MESSAGEDISPATCHER_H

#include "../Entity/base_game_entity.h"
#include "../Entity/entity_manager.h"
#include "telegram.h"
#include <set>

// to make life easier...
#define Dispatcher MessageDispatcher::Instance()

// to make code easier to read
constexpr double SEND_MSG_IMMEDIATELY = 0.0;
constexpr int NO_ADDITIONAL_INFO = 0;
constexpr int SENDER_ID_IRRELEVANT = -1;

class MessageDispatcher {
  private:
	std::set<Telegram> m_DelayedMessages;
	void Discharge(BaseGameEntity* pReciever, const Telegram& message);
	MessageDispatcher() = default;

	// copy ctor and assignment should be private
	MessageDispatcher(const MessageDispatcher&);
	MessageDispatcher& operator=(const MessageDispatcher&);

  public:
	static MessageDispatcher* Instance();

	void DispatchMessage(double delay, int sender, int reciever, int message, void* extraInfo);

	void DispatchDelayedMessages();
};

#endif // MESSAGEDISPATCHER_H
