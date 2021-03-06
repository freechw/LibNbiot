/******************************************************************************
 * File: transition.cpp
 * Author: Edgar Hindemith
 * This file is part of the 'Simple Statemachine for Embedded Systems'
 * see https://github.com/edgar4k/StateMachine
 *
 * Copyright (C) 2016,2018 Edgar Hindemith
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *****************************************************************************/

#include "stmevent.h"
#include "state.h"
#include "choice.h"
#include "transition.h"

Transition::Transition() :
    target(nullptr),
    trAction(),
    eventType(StmEvent::Invalid)
{

}

void Transition::trigger(StmEvent& e)
{
    action(e);
    if(nullptr != target)
    {
        if(NodeTypeTransition == target->type())
        {
            static_cast<Transition*>(target)->trigger(e);
        }
        else if (NodeTypeGuard == target->type())
        {
            static_cast<Choice*>(target)->execute(e);
        }
        else if (NodeTypeState == target->type())
        {
            static_cast<State*>(target)->enter(e);
        }
        else
        {
            e.setEventState(StmEvent::Aborted);
        }
    }
    else
    {
        e.setEventState(StmEvent::Aborted);
    }
}

