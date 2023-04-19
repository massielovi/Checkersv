//
// Created by massi on 19/04/2023.
//

#ifndef CHECKERSV_2_PAWN_HPP
#define CHECKERSV_2_PAWN_HPP

#include <SFML/Graphics.hpp>

//Type of declaration of Players
enum OwningPlayer{
    NOBODY,
    HUMAN,
    COMPUTER,
};

//Sequence of play
OwningPlayer otherPlayer(OwningPlayer current_player){
    if (current_player == HUMAN)
        return COMPUTER;
    return HUMAN;
}

enum PawnLevel{
    normal,
    king,
};

class Pawn{
public:
    OwningPlayer owner;
    PawnLevel level = normal;
    float x,y;
    sf::Vector2i coordinates;

    //Positions
    Pawn(int coord_x, int coord_y, float x_, float y_, OwningPlayer owner_): owner(owner_), x(x_), y(y_){
        coordinates.x = coord_x;
        coordinates.y = coord_y;
    }
    //It could be erased
    void lightUp();//to get better up level
};

#endif //CHECKERSV_2_PAWN_HPP
