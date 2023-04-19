#include <SFML/Graphics.hpp>
#include <iostream>
#include <algorithm>
#include <vector>
#include <memory>
#include "pawn.hpp"

using namespace std;

//alphabeta status
const int plus_infty = 10000;
const int minus_infty = -10000;
//board
const float board_size = 800;
const float field_size = 77.5;
const float border_size = 91;
//Pawns
const int number_paws =24;


enum MoveType{
    INVALID,
    NORMAL,
    BEAT,
    MULTI_BEAT,
};
struct Move{
    Move() = default;
    Move(sf::Vector2i start_, sf::Vector2i finish_, MoveType type_): start(start_), finish(finish_), type(type_){};
    sf::Vector2i start;
    sf::Vector2i finish;
    MoveType type;
};
struct Node{
    Node(Move move_, int value_): move(move_), value(value_){};
    Move move;
    int value;
};
void delay(int miliseconds){
    sf::Clock clock;
    clock.restart();
    while(1){
        if(clock.getElapsedTime().asMilliseconds() > miliseconds)
            break;
    }
}

class Board{
public:

    shared_ptr<Pawn> field[8][8]= {nullptr};
    //Cant Pawns
    vector<weak_ptr<Pawn>> pawn_vector;
    vector<weak_ptr<Pawn>> player_pawns[2];
    bool beat_possible [2] = {false};

    Board(){
        //Number of pawn and initialization for shapes and movement
        pawn_vector.reserve(number_paws);
        float new_x, new_y;
        //Enum called
        OwningPlayer new_player;
        shared_ptr<Pawn> new_ptr;
        for (int i = 0; i < 8; ++i){
            for (int j = 0; j < 8; ++j){
                if (i%2 == j%2){
                    if (j < 3 || j > 4){
                        //Create every position and board to every player
                        new_x = border_size + i * field_size + 5;
                        new_y = border_size + (7-j) * field_size + 5;
                        if (j < 3)
                            new_player = HUMAN;
                        else if (j > 4)
                            new_player = COMPUTER;
                        new_ptr = std::make_shared<Pawn>(i, j, new_x, new_y, new_player);
                        field[i][j] = new_ptr;
                        pawn_vector.push_back(weak_ptr<Pawn>(new_ptr));
                        //Separate for every player a pawns to rest next
                        getVector(new_player).push_back(std::weak_ptr<Pawn>(new_ptr));
                    }
                }
            }
        }
    }
    ~Board(){}
    Board(const Board& copied){
        //Simple copied od board, it uses when calculates possibilities
        for (int i = 0; i < 8; ++i){
            for (int j = 0; j < 8; ++j){
                if (copied.field[i][j]){
                    shared_ptr<Pawn> new_ptr = shared_ptr<Pawn>(new Pawn(*copied.field[i][j]));
                    field[i][j] = new_ptr;
                    pawn_vector.push_back(weak_ptr<Pawn>(new_ptr));

                    getVector(new_ptr->owner).push_back(std::weak_ptr<Pawn>(new_ptr));
                }
            }
        }
    }
    void print(){
        for (int y = 7; y > -1 ; --y){
            for (int x = 0; x < 8; ++x){
                auto printed_pawn = getPawn(sf::Vector2i(x, y));
                if (printed_pawn){
                    if (printed_pawn->owner == HUMAN)
                        std::cerr << 'O';
                    else
                        std::cerr << 'X';
                }
                else
                    std::cerr << ' ';
            }
            std::cerr << '\n';
        }
        std::cerr << '\n';
    }

    vector<weak_ptr<Pawn>>& getVector (OwningPlayer player){
        if (player == HUMAN)
            return player_pawns[0];
        else
            return player_pawns[1];
    }
    shared_ptr<Pawn> getPawn(const sf::Vector2i& coords){
        //Found the possibility to put Pawn depends on empty coords
        if (field[coords.x][coords.y] != nullptr)
            return field[coords.x][coords.y];
        else
            return nullptr;
    }

    MoveType checkMove(sf::Vector2i& start, sf::Vector2i& finish){
        //Every disable movement is writes here
        MoveType result = INVALID;
        if(finish.x >= 0 && finish.x <= 7 && finish.y >= 0 && finish.y <=7){
            if(std::shared_ptr<Pawn> pawn = getPawn(start)){
                int direction = 1;
                if (pawn->owner == COMPUTER)
                    direction = -1;
                if (finish.y == start.y + direction){
                    if (finish.x == start.x + 1 || finish.x == start.x - 1){
                        if (!getPawn(finish)){
                            if(!getBeatPossible(pawn->owner)){
                                result = NORMAL;
                            }
                        }
                    }
                }
                else if (finish.y == start.y + 2*direction){
                    if (finish.x == start.x + 2 || finish.x == start.x - 2){
                        if (!getPawn(finish)){
                            sf::Vector2i beaten_pawn(start.x + (finish.x - start.x)/2, start.y + direction);
                            if (getPawn(beaten_pawn)){
                                if (getPawn(beaten_pawn)->owner == otherPlayer(pawn->owner)){
                                    result = BEAT;
                                }
                            }
                        }
                    }
                }
            }
        }
        return result;
    }

    vector<Move>* getAvailibleMoves(OwningPlayer player, const shared_ptr<Pawn> pawn){
        vector<Move>* move_vector = new vector<Move>;
        sf::Vector2i start, finish;
        int direction = 1;
        if (pawn){
            if (player == COMPUTER)
                direction = -1;
            start = pawn->coordinates;
            for (int k: {1,2}){
                for (int l: {-1,1}){
                    finish = start + sf::Vector2i(l*k, k*direction);
                    // std::cerr << start.x << ' ' << start.y << ' ' << finish.x <<  ' ' << finish.y;
                    MoveType result = checkMove(start, finish);
                    if (result != INVALID){
                        Move new_move = Move(start, finish, result);
                        // std::cerr << " valid move";
                        move_vector->push_back(new_move);
                    }
                    // std::cerr << '\n';
                }
            }
        }
        return move_vector;
    }
    vector<Move>* getAvailibleMoves(OwningPlayer player){
        //Movements send to a vector to keep
        vector<Move>* move_vector = new vector<Move>;
        for (auto pawn_ptr: getVector(player)){
            if (auto pawn = pawn_ptr.lock()){
                auto new_moves = getAvailibleMoves(player, pawn);
                if(!new_moves->empty())
                    move_vector->insert(move_vector->end(), new_moves->begin(), new_moves->end());
                delete new_moves;
            }
        }
        return move_vector;
    }

    bool& getBeatPossible(OwningPlayer player){
        if (player == COMPUTER)
            return beat_possible[1];
        return beat_possible[0];
    }
    void resolveBeating(OwningPlayer player){
        getBeatPossible(player) = false;
        std::vector<Move>* move_vector = getAvailibleMoves(player);
        for (auto tested_move: *move_vector){
            if (tested_move.type == BEAT)
                getBeatPossible(player) = true;
        }
    }

    int setPawn(const sf::Vector2i& coords, const std::shared_ptr<Pawn>& new_ptr){
        field[coords.x][coords.y] = new_ptr;
        return 0;
    }
    shared_ptr<Pawn> movePawn(sf::Vector2i start, sf::Vector2i finish, MoveType type){

        if (auto pawn = getPawn(start)){
            int direction = 1;
            if (pawn->owner == COMPUTER)
                direction = -1;
            if(type == BEAT){
                sf::Vector2i beaten_pawn(start.x + (finish.x - start.x)/2, start.y + direction);
                getPawn(beaten_pawn).reset();
                setPawn(beaten_pawn, nullptr);
            }
            setPawn(start, nullptr);
            setPawn(finish, pawn);
            pawn->coordinates = finish;
            resolveBeating(pawn->owner);

            return pawn;
        }
        return nullptr;
    }
    shared_ptr<Pawn> movePawn(const Move& move){
        return movePawn(move.start, move.finish, move.type);
    }

    int getScore(OwningPlayer player){
        int score = 0;
        for (auto pawn_weak: getVector(player)){
            if (auto pawn = pawn_weak.lock()){
                vector<Move>* move_vector = new std::vector<Move>;
                int x = pawn->coordinates.x;
                int y = pawn->coordinates.y;
                score += 10;
                if (player == HUMAN){
                    if (y == 2 || y == 3)
                        score += 1;
                    else if (y == 4 || y == 5)
                        score += 3;
                    else if (y == 6 || y == 7)
                        score += 5;
                }
                else{
                    if (y == 5 || y == 4)
                        score += 1;
                    else if (y == 3 || y == 2)
                        score += 3;
                    else if (y == 1 || y == 0)
                        score += 5;
                }
                if ((x == 0 || x == 7) && (y == 0 || y == 7))
                    score += 2;
                else if ((x == 1 || x == 6) && (y == 1 || y == 6))
                    score += 1;
                move_vector = getAvailibleMoves(player, pawn);
                if (!move_vector->empty()){
                    for (auto tested_move: *move_vector){
                        if (tested_move.type == BEAT)
                            score += 30;
                    }
                }
                delete move_vector;
            }
        }
        return score;
    }

    OwningPlayer checkWin(OwningPlayer player){
        resolveBeating(player);
        OwningPlayer winner = NOBODY;
        vector<Move>* availible_moves;

        availible_moves = getAvailibleMoves(player);
        if (availible_moves->empty()){
            winner = otherPlayer(player);
        }
        else {
            int pawn_count = 0;
            for (auto checked_pawn: getVector(player)){
                if (!checked_pawn.expired())
                    ++pawn_count;
            }
            if(!pawn_count){
                winner = otherPlayer(player);
            }
        }
        delete availible_moves;
        return winner;
    }
    //std::shared_ptr<Pawn> field[8][8]= {nullptr};
    //std::vector<std::weak_ptr<Pawn>> pawn_vector;


};


class Game{
public:
    //Create every part to draw in SFML
    sf::Texture textures[5];//Parts of checkers
    sf::Sprite sprites[5];
    sf::Image icon;
    sf::RenderWindow window;
    OwningPlayer players[2] = {HUMAN, COMPUTER};
    Board game_board;
    //Who starts to play?
    OwningPlayer active_player = HUMAN;

    Game(){
        //pictures needed to interface
        //icon initialization
        icon.loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\red.png");
        //Every part of checkers
        // texture initialization
        textures[0].loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\board.jpg");
        textures[1].loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\black.png");
        textures[2].loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\red.png");
        textures[3].loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\black_king.png");
        textures[4].loadFromFile("C:\\VII-Semester\\2.IA\\Checkersv\\graphics\\red_king.png");

        //Forms of every Image loaded
        sprites[0].setTexture(textures[0]);
        for (int i = 1; i < 5; ++i){
            sprites[i].setTexture(textures[i]);
            sprites[i].setScale(0.6,0.6);
        }
    }

    void view(){
        window.clear();
        //draw the board
        window.draw(sprites[0]);
        int sprite_number;
        //draw the pawns
        for(const auto pawn_ptr: game_board.pawn_vector){
            if (auto drawn_pawn = pawn_ptr.lock()){
                if (drawn_pawn->owner == HUMAN)
                    sprite_number = 1;
                else
                    sprite_number = 2;
                sprites[sprite_number].setPosition(drawn_pawn->x, drawn_pawn->y);
                window.draw(sprites[sprite_number]);
            }
        }
        window.display();
    }
    void start(){
        //Create our interface window
        //Board atributes
        window.create(sf::VideoMode(board_size, board_size), "CHEEMS Checkers");

        window.setIcon(icon.getSize().x, icon.getSize().y, icon.getPixelsPtr());
        view();
    }

    //Computer Play
    int alphabeta(Board& current_board, Move& best_move, int depth, OwningPlayer player, int alpha, int beta){
        // std::cerr << "start poziom " << depth << '\n';
        int value;
        // current_board.print();
        if (depth == 0){ //or node is a terminal node
            value = current_board.getScore(COMPUTER) - current_board.getScore(HUMAN);
            // std::cerr << " return " << value << '\n';
            return value;
        }

        std::vector<Move>* possible_moves = current_board.getAvailibleMoves(player);
        std::vector<Board>* possible_boards = new std::vector<Board>(possible_moves->size(), current_board);
        // std::cerr << possible_moves->size() << "dostępnych ruchów\n";
        for (unsigned int i = 0; i < possible_moves->size(); ++i){
            possible_boards->at(i).movePawn(possible_moves->at(i));
        }
        if (player == COMPUTER){
            for (unsigned int i = 0; i < possible_boards->size(); ++i){
                value = alphabeta(possible_boards->at(i), best_move, depth-1, HUMAN, alpha, beta);
                alpha = std::max(alpha, value);
                if (alpha == value && depth == 6)
                    best_move = possible_moves->at(i);
                if (alpha >= beta){
                    // std::cerr << "alpha cut";
                    break;
                }
            }
            return alpha;
        }
        else{
            for (unsigned int i = 0; i < possible_boards->size(); ++i){
                beta = std::min(beta, alphabeta(possible_boards->at(i), best_move, depth-1, COMPUTER, alpha, beta));
                if (alpha >= beta){
                    // std::cerr << "beta cut";
                    break;
                }
            }
            return beta;
        }
        delete possible_moves;
        delete possible_boards;
        // std::cerr << "stop poziom " << depth << "- " << value << '\n';
    }
    void executeMove(sf::Vector2i& start, sf::Vector2i& finish, MoveType type){
        if(auto pawn = game_board.movePawn(start, finish, type)){
            float distance_x = ((finish.x - start.x) * field_size) / 10;
            float distance_y = ((finish-start).y * field_size) / 10;
            for (int i = 0; i < 10; ++i){
                pawn->x += distance_x;
                pawn->y -= distance_y;
                delay(20);
                view();
            }
            view();
        }

    }
    int computerMove(){
        //How the computer is going to play
        Move computer_move;
        sf::Clock clock;
        clock.restart();
        //Implementation of alphabeta algoritm
        alphabeta(game_board, computer_move, 6, COMPUTER, minus_infty, plus_infty);
        cerr << clock.getElapsedTime().asMilliseconds();
        executeMove(computer_move.start, computer_move.finish, computer_move.type);
        return 0;
    }

    //User Play
    bool pollEvents(sf::Vector2i& mouse_position){
        sf::Event event;
        while (window.pollEvent(event)){
            if (event.type == sf::Event::Closed){
                window.close();
                return false;
            }
            if (event.type == sf::Event::MouseButtonPressed){
                if (event.mouseButton.button == sf::Mouse::Left){
                    mouse_position.x = event.mouseButton.x;
                    mouse_position.y = event.mouseButton.y;
                    return true;
                }
            }
        }
        return false;
    }
    int manualMove(OwningPlayer player){
        sf::Vector2i mouse_position, start, finish;
        sf::Vector2i* updated_vector;
        std::shared_ptr<Pawn> active_pawn;
        bool mouse_pressed=false;
        while (window.isOpen()){
            mouse_pressed = pollEvents(mouse_position);
            if (mouse_pressed){
                if(mouse_position.x > border_size && mouse_position.x < board_size - border_size &&
                   mouse_position.y > border_size && mouse_position.y < board_size - border_size){
                    if (!active_pawn){
                        updated_vector = &start;
                    }
                    else{
                        updated_vector = &finish;
                    }
                    updated_vector->x = (mouse_position.x - border_size) / field_size;
                    updated_vector->y = (mouse_position.y - border_size) / field_size;
                    updated_vector->y = 7 - updated_vector->y;
                    if (active_pawn){
                        //std::cerr << start.x << start.y << '-' << finish.x << finish.y << '\n';
                        if(active_pawn->owner == player){
                            MoveType result = game_board.checkMove(start, finish);
                            if (result != INVALID){
                                executeMove(start, finish, result);
                                return 0;
                            }
                        }
                        active_pawn = nullptr;
                    }
                    else {
                        active_pawn = game_board.getPawn(start);
                    }
                }
            }
        }
        return 1;
    }

    int getMove(OwningPlayer player){
        //Depend on the turn in the play
        game_board.resolveBeating(player);
        if (player == COMPUTER)
            return computerMove();
        else
            return manualMove(HUMAN);
    }

    void play(){
        //Start interactive play
        Move computer_move;
        OwningPlayer winner = NOBODY;
        while(winner == NOBODY){
            if(getMove(active_player))
                break;
            // std::cerr << alphabeta(game_board, computer_move, 2, COMPUTER, minus_infty, plus_infty);
            active_player = otherPlayer(active_player);
            winner = game_board.checkWin(active_player);
        }
        //Print the status finish game
        if (winner == HUMAN)
            std::cout << "You are the WINNER!!! \\O_0/\n";
        else if (winner == COMPUTER)
            std::cout << "You are the LOOOSER :0 \n";
    }
    //sf::RenderWindow window;
    //sf::Texture textures[5];
    //sf::Sprite sprites[5];
    //sf::Image icon;

    //Board game_board;
    //OwningPlayer active_player = HUMAN;
};




int main(int argc, char const *argv[])
{
    Game my_game;
    my_game.start();
    my_game.play();

    return 0;
}
