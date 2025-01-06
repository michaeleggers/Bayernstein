//
// Created by benek on 10/18/24.
//

#ifndef MESSAGETYPE_H
#define MESSAGETYPE_H

#include <string>

enum message_type
{
    Attack,
    Collision,
    RayHit
};

inline std::string MessageToString(int message)
{
    switch ( message )
    {
    case 0:
    {
        return "Attack";
    }
    break;
    case 1:
    {
        return "Collision";
    }
    break;
    case 2:
    {
        return "RayHit";
    }
    break;

    default:
    {
        return "Not recognized!";
    }
    }
}

#endif // MESSAGETYPE_H
