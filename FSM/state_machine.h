//
// Created by benek on 10/14/24.
//

#ifndef STATEMACHINE_H
#define STATEMACHINE_H

#include "istate.h"
#include <assert.h>
#include <typeinfo>

template <class entity_type> class StateMachine {
  private:
    entity_type*        m_pOwner;
    State<entity_type>* m_pCurrentState;
    State<entity_type>* m_pPreviousState;
    State<entity_type>* m_pGlobalState;

  public:
    explicit StateMachine(entity_type* owner)
        : m_pOwner(owner),
          m_pCurrentState(nullptr),
          m_pPreviousState(nullptr),
          m_pGlobalState(nullptr) {};

    virtual ~StateMachine() = default;

    // use these methods to initialize the FSM
    void SetCurrentState(State<entity_type>* state) {
        m_pCurrentState = state;
    }
    void SetGlobalState(State<entity_type>* state) {
        m_pGlobalState = state;
    }
    void SetPreviousState(State<entity_type>* state) {
        m_pPreviousState = state;
    }

    // call this to update the FSM
    void Update() const {

        // if a global state exists, call its execute method, else do nothing
        if ( m_pGlobalState ) {
            m_pGlobalState->Execute(m_pOwner);
        }

        // same for the current state
        if ( m_pCurrentState ) {
            m_pCurrentState->Execute(m_pOwner);
        }
    }

    // change to a new state
    void ChangeState(State<entity_type>* pNewState) {
        if ( m_pCurrentState == pNewState ) {
            return;
        }
        assert(pNewState && "<StateMachine::ChangeState>:trying to assign null state to current");

        // keep a record of the previous state
        m_pPreviousState = m_pCurrentState;

        // call the exit method of the existing state
        m_pCurrentState->Exit(m_pOwner);
        // change state to the new state
        m_pCurrentState = pNewState;

        // call the entry method of the new state
        m_pCurrentState->Enter(m_pOwner);
    }

    // change state back to the previous state
    void RevertToPreviousState() {
        ChangeState(m_pPreviousState);
    }

    // returns true if the current state's type is equal to the type of the
    // class passed as a parameter.
    bool IsInState(const State<entity_type>& state) const {
        if ( typeid(*m_pCurrentState) == typeid(state) ) {
            return true;
        }
        return false;
    }

    static bool IsSameState(const State<entity_type>& state1, const State<entity_type>& state2) {
        return typeid(state1) == typeid(state2);
    }

    State<entity_type>* CurrentState() const {
        return m_pCurrentState;
    }
    State<entity_type>* GlobalState() const {
        return m_pGlobalState;
    }
    State<entity_type>* PreviousState() const {
        return m_pPreviousState;
    }

    bool HandleMessage(const Telegram& message) const {
        // first see if the current state is valid and that it can handle
        // the message
        if ( m_pCurrentState && m_pCurrentState->OnMessage(m_pOwner, message) ) {
            return true;
        }

        // if not, and if a global state has been implemented, send
        // the message to the global state
        if ( m_pGlobalState && m_pGlobalState->OnMessage(m_pOwner, message) ) {
            return true;
        }

        return false;
    }
};

#endif // STATEMACHINE_H
