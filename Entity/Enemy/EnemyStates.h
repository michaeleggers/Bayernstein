//
// Created by benek on 10/14/24.
//

#ifndef ENEMYSTATES_H
#define ENEMYSTATES_H
#include "../FSM/State.h"
#include "Enemy.h"



class EnemyIdle : public State<Enemy> {
  private:
	EnemyIdle() = default;
	// copy ctor and assignment should be private
	EnemyIdle(const EnemyIdle &);
	EnemyIdle &operator=(const EnemyIdle &);

  public:
	// this is a singleton
	static EnemyIdle *Instance();

	void Enter(Enemy *pEnemy) override;

	void Execute(Enemy *pEnemy) override;

	void Exit(Enemy *pEnemy) override;

	//	virtual bool OnMessage(Enemy* agent, const Telegram& msg);
};

class EnemyRunning : public State<Enemy> {
  private:
	EnemyRunning() = default;
	// copy ctor and assignment should be private
	EnemyRunning(const EnemyRunning &);
	EnemyRunning &operator=(const EnemyRunning &);

  public:
	// this is a singleton
	static EnemyRunning *Instance();

	void Enter(Enemy *pEnemy) override;

	void Execute(Enemy *pEnemy) override;

	void Exit(Enemy *pEnemy) override;

	//	virtual bool OnMessage(Enemy* agent, const Telegram& msg);
};

class EnemyAttacking : public State<Enemy> {
  private:
	EnemyAttacking() = default;
	// copy ctor and assignment should be private
	EnemyAttacking(const EnemyAttacking &);
	EnemyAttacking &operator=(const EnemyAttacking &);

  public:
	// this is a singleton
	static EnemyAttacking *Instance();

	void Enter(Enemy *pEnemy) override;

	void Execute(Enemy *pEnemy) override;

	void Exit(Enemy *pEnemy) override;

	//	virtual bool OnMessage(Enemy* agent, const Telegram& msg);
};



#endif // ENEMYSTATES_H
