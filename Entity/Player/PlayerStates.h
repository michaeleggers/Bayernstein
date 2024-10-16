//
// Created by benek on 10/14/24.
//

#ifndef PLAYERSTATES_H
#define PLAYERSTATES_H
#include "../FSM/State.h"
#include "Player.h"



class PlayerIdle : public State<Player> {
  private:
	PlayerIdle() = default;
	// copy ctor and assignment should be private
	PlayerIdle(const PlayerIdle &);
	PlayerIdle &operator=(const PlayerIdle &);

  public:
	// this is a singleton
	static PlayerIdle *Instance();

	void Enter(Player *pPlayer) override;

	void Execute(Player *pPlayer) override;

	void Exit(Player *pPlayer) override;

	//	virtual bool OnMessage(Player* agent, const Telegram& msg);
};

class PlayerRunning : public State<Player> {
  private:
	PlayerRunning() = default;
	// copy ctor and assignment should be private
	PlayerRunning(const PlayerRunning &);
	PlayerRunning &operator=(const PlayerRunning &);

  public:
	// this is a singleton
	static PlayerRunning *Instance();

	void Enter(Player *pPlayer) override;

	void Execute(Player *pPlayer) override;

	void Exit(Player *pPlayer) override;

	//	virtual bool OnMessage(Player* agent, const Telegram& msg);
};

class PlayerAttacking : public State<Player> {
  private:
	PlayerAttacking() = default;
	// copy ctor and assignment should be private
	PlayerAttacking(const PlayerAttacking &);
	PlayerAttacking &operator=(const PlayerAttacking &);

  public:
	// this is a singleton
	static PlayerAttacking *Instance();

	void Enter(Player *pPlayer) override;

	void Execute(Player *pPlayer) override;

	void Exit(Player *pPlayer) override;

	//	virtual bool OnMessage(Player* agent, const Telegram& msg);
};


#endif // PLAYERSTATES_H
