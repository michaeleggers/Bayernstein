//
// Created by benek on 10/14/24.
//

#include "EnemyStates.h"
#include <iostream>

EnemyIdle *EnemyIdle::Instance() {
	static EnemyIdle instance;

	return &instance;
}

void EnemyIdle::Enter(Enemy *pEnemy) { std::cout << "Enemy entered Idle State\n"; }

void EnemyIdle::Execute(Enemy *pEnemy) { std::cout << "Enemy is executing Idle State\n"; }
void EnemyIdle::Exit(Enemy *pEnemy) { std::cout << "Enemy is exiting Idle State\n"; }

EnemyRunning *EnemyRunning::Instance() {
	static EnemyRunning instance;

	return &instance;
}

void EnemyRunning::Enter(Enemy *pEnemy) { std::cout << "Enemy entered Running State\n"; }

void EnemyRunning::Execute(Enemy *pEnemy) { std::cout << "Enemy is executing Running State\n"; }
void EnemyRunning::Exit(Enemy *pEnemy) { std::cout << "Player is exiting Running State\n"; }

EnemyAttacking *EnemyAttacking::Instance() {
	static EnemyAttacking instance;

	return &instance;
}

void EnemyAttacking::Enter(Enemy *pEnemy) { std::cout << "Enemy entered Running State\n"; }

void EnemyAttacking::Execute(Enemy *pEnemy) { std::cout << "Enemy is executing Running State\n"; }
void EnemyAttacking::Exit(Enemy *pEnemy) { std::cout << "Player is exiting Running State\n"; }