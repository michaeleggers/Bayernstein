//
// Created by benek on 10/14/24.
//

#ifndef ENEMYSTATES_H
#define ENEMYSTATES_H
#include "../FSM/istate.h"
#include "g_enemy.h"

class EnemyIdle : public State<Enemy> {
  private:
	EnemyIdle() = default;
	// copy ctor and assignment should be private
	EnemyIdle(const EnemyIdle&);
	EnemyIdle& operator=(const EnemyIdle&);

  public:
	// this is a singleton
	static EnemyIdle* Instance();

	void Enter(Enemy* pEnemy) override;

	void Execute(Enemy* pEnemy) override;

	void Exit(Enemy* pEnemy) override;

	bool OnMessage(Enemy* agent, const Telegram& telegram) override;
};

class EnemyAttacking : public State<Enemy> {
  private:
	EnemyAttacking() = default;
	// copy ctor and assignment should be private
	EnemyAttacking(const EnemyAttacking&);
	EnemyAttacking& operator=(const EnemyAttacking&);

  public:
	// this is a singleton
	static EnemyAttacking* Instance();

	void Enter(Enemy* pEnemy) override;

	void Execute(Enemy* pEnemy) override;

	void Exit(Enemy* pEnemy) override;

	bool OnMessage(Enemy* agent, const Telegram& telegram) override;
};

class EnemyDead : public State<Enemy> {
  private:
	EnemyDead() = default;

	// copy ctor and assignment should be private
	EnemyDead(const EnemyDead&);
	EnemyDead& operator=(const EnemyDead&);

  public:
	// copy ctor and assignment should be private
	// this is a singleton
	static EnemyDead* Instance();

	void Enter(Enemy* pEnemy) override;

	void Execute(Enemy* pEnemy) override;

	void Exit(Enemy* pEnemy) override;

	bool OnMessage(Enemy* agent, const Telegram& telegram) override;
};

class EnemyWander : public State<Enemy> {
  private:
    EnemyWander() = default;

    // copy ctor and assignment should be private
    EnemyWander(const EnemyWander&);
    EnemyWander& operator=(const EnemyWander&);

  public:
    // copy ctor and assignment should be private
    // this is a singleton
    static EnemyWander* Instance();

    void Enter(Enemy* pEnemy) override;

    void Execute(Enemy* pEnemy) override;

    void Exit(Enemy* pEnemy) override;

    bool OnMessage(Enemy* agent, const Telegram& telegram) override;
};

class EnemyPatrol : public State<Enemy> {
  private:
    EnemyPatrol() = default;

    // copy ctor and assignment should be private
    EnemyPatrol(const EnemyPatrol&);
    EnemyPatrol& operator=(const EnemyPatrol&);

  public:
    // copy ctor and assignment should be private
    // this is a singleton
    static EnemyPatrol* Instance();

    void Enter(Enemy* pEnemy) override;

    void Execute(Enemy* pEnemy) override;

    void Exit(Enemy* pEnemy) override;

    bool OnMessage(Enemy* agent, const Telegram& telegram) override;
};

#endif // ENEMYSTATES_H
