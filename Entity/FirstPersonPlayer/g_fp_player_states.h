//
// Created by me on 10/14/24.
//

#ifndef _FP_PLAYERSTATES_H_
#define _FP_PLAYERSTATES_H_

#include "g_fp_player.h"
#include "../../FSM/istate.h"

class FirstPersonPlayerIdle : public State<FirstPersonPlayer> {
  private:
	FirstPersonPlayerIdle() = default;
	// copy ctor and assignment should be private
	FirstPersonPlayerIdle(const FirstPersonPlayerIdle&);
	FirstPersonPlayerIdle& operator=(const FirstPersonPlayerIdle&);

  public:
	// this is a singleton
	static FirstPersonPlayerIdle* Instance();

	void Enter(FirstPersonPlayer* pPlayer) override;

	void Execute(FirstPersonPlayer* pPlayer) override;

	void Exit(FirstPersonPlayer* pPlayer) override;

	bool OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) override;
};

class FirstPersonPlayerRunning : public State<FirstPersonPlayer> {
  private:
	FirstPersonPlayerRunning() = default;
	// copy ctor and assignment should be private
	FirstPersonPlayerRunning(const FirstPersonPlayerRunning&);
	FirstPersonPlayerRunning& operator=(const FirstPersonPlayerRunning&);

  public:
	// this is a singleton
	static FirstPersonPlayerRunning* Instance();

	void Enter(FirstPersonPlayer* pPlayer) override;

	void Execute(FirstPersonPlayer* pPlayer) override;

	void Exit(FirstPersonPlayer* pPlayer) override;

	bool OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) override;
};

class FirstPersonPlayerAttacking : public State<FirstPersonPlayer> {
  private:
	FirstPersonPlayerAttacking() = default;
	// copy ctor and assignment should be private
	FirstPersonPlayerAttacking(const FirstPersonPlayerAttacking&);
	FirstPersonPlayerAttacking& operator=(const FirstPersonPlayerAttacking&);

  public:
	// this is a singleton
	static FirstPersonPlayerAttacking* Instance();

	void Enter(FirstPersonPlayer* pPlayer) override;

	void Execute(FirstPersonPlayer* pPlayer) override;

	void Exit(FirstPersonPlayer* pPlayer) override;

	bool OnMessage(FirstPersonPlayer* agent, const Telegram& telegram) override;
};

#endif // _FP_PLAYERSTATES_H_

