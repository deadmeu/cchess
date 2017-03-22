#include <stdio.h>
#include <stdlib.h>
#include "chess.h"

#define MAX_LINE_LIM 16
#define ROWS 8
#define COLS 8
#define WHITE 1
#define BLACK 2
#define EMPTY 0

int main(int argc, char** args) {
    Game* game = malloc(sizeof(Game));
    create_board(game);
    // set_piece(7, 3, 3, game->board);
    do {
        print_board(game->board);
        game->curPlayer = WHITE;
        handle_input(game);
        process_game(game);
    } while(game->gameOver != 1);
    // add free_mem fn?
    return 0;
}

void handle_input(Game* game) {
    char invalidInput = 0;
    int argc = 0;
    int x1 = 0;
    int y1 = 0;
    int x2 = 0;
    int y2 = 0;
    int* curArg = &x1;  // first expected argument is a col value
    char buffer[256];
    printf("Move> ");
    fflush(stdout);
    if (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        for (int i = 0; i < MAX_LINE_LIM + 1; i++) {
            if (i == MAX_LINE_LIM && buffer[i] != '\0') {
                // input too large toggle invalid input state
                invalidInput = 1;
                break;
            }
            if (buffer[i] == '\n') {
                if (i == 0) {
                    invalidInput = 1;
                }
                break;
            }
            if (buffer[i] == ' ' || buffer[i] == ',') {
                argc++;
                switch (argc) {
                    case 1: curArg = &y1; break;
                    case 2: curArg = &x2; break;
                    case 3: curArg = &y2; break;
                    default: invalidInput = 1;  // more arguments than expected
                }
            } else {
                if (buffer[i] >= '0' && buffer[i] <= '9') {
                    *curArg *= 10;  //for multiple digit numbers (not normally needed)
                    *curArg += (buffer[i] - '0');
                } else {
                    invalidInput = 1;
                }
            }
        }
    } else {
        // NULL or EOF- throw error here
        error(1);
    }
    if (curArg != &y2) {
        invalidInput = 1;
    }
    if (!(is_move_legal(x1, y1, x2, y2, game))) {
        invalidInput = 1;
    }

    // if input was successfully read, parse it
    if (!invalidInput) {
        // here we would parse x1,x2,y1,y2 into a game move
        game->startX = x1;
        game->startY = y1;
        game->endX = x2;
        game->endY = y2;
    } else {
        // input was not successfully read, try again
        handle_input(game);
    }
}

int is_move_legal(int x1, int y1, int x2, int y2, Game* game) {
    // check for legal board bounds
    if (!(x1 >= 1 && x1 <= COLS &&
        x2 >= 1 && x2 <= COLS &&
        y1 >= 1 && y1 <= ROWS &&
        y2 >= 1 && y2 <= ROWS)) {
        return 0;
    }
    // check if piece being moved is owned by the current player
    if (get_piece_color(get_piece(x1, y1, game->board)) != game->curPlayer) {
        return 0;
    }
    // check to make sure we aren't capturing our own piece (or not moving at all)
    if (!(get_piece_color(get_piece(x2, y2, game->board)) != game->curPlayer)) {
        return 0;
    }
    // check to make sure the piece is moving/attacking correctly
    if (!(is_piece_move_legal(x1, y1, x2, y2, game->board))) {
        return 0;
    }

    return 1;
}

int is_piece_move_legal(int x1, int y1, int x2, int y2, int** board) {
    char thisPieceType = get_piece_type(get_piece(x1, y1, board));
    int thisPieceColor = get_piece_color(get_piece(x1, y1, board));
    // int thatPieceColor = get_piece_color(get_piece(x2, y2, board));
    int isLegal = 0;
    switch (thisPieceType) {
        case 'P':   // Pawn
            isLegal = check_valid_move_pawn(x1, y1, x2, y2, thisPieceColor, board); break;
        case 'N':   // Knight
            isLegal = 0;
    }
    return isLegal;
}

int check_valid_move_pawn(int x1, int y1, int x2, int y2, int thisPieceColor, int** board) {
    // check if moved backwards (illegal)
    if ((thisPieceColor == WHITE && (y2 - y1) < 1) ||
        (thisPieceColor == BLACK && (y1 - y2) < 1)) {
        return 0;
    }

    // check if only moved forwards by one space (legal)
    if (x2 - x1 == 0 && ((thisPieceColor == WHITE && (y2 - y1) == 1) ||
                        (thisPieceColor == BLACK && (y1 - y2) == 1)) &&
                        !(check_for_collision(x1, y1, x2, y2, board))) {
        return 1;
    }

    // check if on second (relative) rank for first double move
    if (x2 - x1 == 0 && ((thisPieceColor == WHITE && (y2 - y1) > 1) ||
                        (thisPieceColor == BLACK && (y1 - y2) > 1)) &&
                        !(check_for_collision(x1, y1, x2, y2, board))) {

        if (thisPieceColor == WHITE && (y2 - y1) == 2 && y1 == 2) {  // is white pawn moving two from rank 2
            return 1;
        }
        if (thisPieceColor == BLACK && (y1 - y2) == 2 && y1 == 7) {  // is black pawn moving two from rank 7
            return 1;
        }
        return 0;
    }

    // check for diagonal attack
    if (x2 != x1) {
        if ((abs(x2 - x1) == 1) && (y2 - y1 == 1) &&
            !(is_square_empty(x2, y2, board))) { // check that piece exists
                return 1;
        }
        return 0;
    }
    return 0;
}

int check_for_collision(int x1, int y1, int x2, int y2, int** board) {
    int deltaX = x2 - x1;
    int deltaY = y2 - y1;
    // movement caused change in x (horizontal / diagonal movement)
    for (int i = 1; i <= deltaX; i++) {
        if (deltaX) {   // deltaX is pos
            if (!(is_square_empty(x1 + i, y1 + (deltaY / deltaX) * i, board))) {
                return 1;
            }
        } else {    // deltaX is neg
            if (!(is_square_empty(x1 - i, y1 + (deltaY / deltaX) * i, board))) {
                return 1;
            }
        }
    }
    // movement caused only a change in y (vertical movement)
    for (int j = 1; j <= deltaY; j++) {
        if (deltaY) {   // deltaY is pos
            if (!(is_square_empty(x1, y1 + j, board))) {
                return 1;
            }
        } else {    // deltaY is neg
            if (!(is_square_empty(x1, y1 - j, board))) {
                return 1;
            }
        }
    }
    return 0;
}

int is_square_empty(int x, int y, int** board) {
    if (get_piece_color(get_piece(x, y, board)) == EMPTY) {
        return 1;
    } else {
        return 0;
    }
}

void process_game(Game* game) {
    move_piece(game->startX, game->startY, game->endX, game->endY, game->board);
}



    // char charBuffer;
    // char invalidInput = 0;
    // int argc = 0;
    // int x1;
    // int y1;
    // int x2;
    // int y2;
    // int* curArg = &x1;  // first expected argument is a file
    // printf("Type Something: ");
    // fflush(stdout);
    // for (int i = 0; i < MAX_LINE_LIM + 1; i++) {
    //     if (i > MAX_LINE_LIM) {
    //         // too much input without a newline provided
    //         printf("rip\n");
    //         invalidInput = 1;
    //         break;
    //     }
    //     charBuffer = fgetc(stdin);
    //     if (feof(stdin) || charBuffer == 0) {
    //         //EOF - throw error
    //     } else if (charBuffer == '\n') {
    //         if (i <= 1) {
    //             invalidInput = 1;
    //         }
    //         break;
    //     } else if (charBuffer == ' ') {
    //         argc++;
    //         switch (argc) {
    //             case 1: curArg = &y1; break;
    //             case 2: curArg = &x2; break;
    //             case 3: curArg = &y2; break;
    //             default: invalidInput = 1;  // more arguments than expected
    //         }
    //     } else {
    //         if (charBuffer >= '0' && charBuffer <= '9') {
    //             *curArg *= 10;     // for multiple digit numbers (not normally needed)
    //             *curArg += (charBuffer - '0');
    //         } else {
    //             invalidInput = 1;
    //         }
    //     }
    // }
    // fflush(stdin);
    // // if input was successfully read, parse it
    // if (!invalidInput) {
    //     // here we would parse x1,x2,y1,y2 into a game move
    // } else {
    //     // input was not successfully read, try again
    //     handle_input(game);
    // }
// }

// Creates the 2d board array
void create_board(Game* game) {
    if (ROWS > 0 && COLS > 0) {
        // make 2d board array
        game->board = malloc(sizeof(int*) * ROWS);
        for (int i = 0; i < ROWS; i++) {
            game->board[i] = malloc(sizeof(int) * COLS);
            for (int j = 0; j < COLS; j++) {
                game->board[i][j] = 0;
            }
        }
        reset_board(game->board);
    } else {
        // throw error here - board size too small
    }
}

// Resets the board to a default (8x8) setup
void reset_board(int** board) {
    const char* defaultPieces[32] = {"A18", "828", "938", "B48",
                                    "C58", "968", "878", "A88",
                                    "717", "727", "737", "747",
                                    "757", "767", "777", "787",
                                    "112", "122", "132", "142",
                                    "152", "162", "172", "182",
                                    "411", "221", "331", "541",
                                    "651", "361", "271", "481"};
    // place all the pieces by iterating over size of defaultPieces array
    for (int i = 0; i < sizeof(defaultPieces) / sizeof(int*); i++) {
        // char - '0' converts a char int to an int
        set_piece(convert_hex_char_to_int(defaultPieces[i][0]),
                    defaultPieces[i][1] - '0',
                    defaultPieces[i][2] - '0', board);
    }
}

int convert_hex_char_to_int(char hexAsChar) {
    int num = (int) strtol((char[]) {hexAsChar, 0}, NULL, 16);
    return num;
}

// Converts a piece char int value into a piece letter type
char get_piece_type(int pieceValue) {
    // if unknown piece value just return whitespace
    if  (!(pieceValue >= 1 && pieceValue <= 0xC)) {
        return ' ';
    }
    char pieceType;
    switch (pieceValue) {
        case 1:
        case 7: pieceType = 'P'; break;
        case 2:
        case 8: pieceType = 'N'; break;
        case 3:
        case 9: pieceType = 'B'; break;
        case 4:
        case 10: pieceType = 'R'; break;
        case 5:
        case 11: pieceType = 'Q'; break;
        case 6:
        case 12: pieceType = 'K'; break;
    }
    return pieceType;
}

// Converts a piece char int value into a piece colour type
int get_piece_color(int pieceValue) {
    // if unknown piece value just return unknown
    if  (!(pieceValue >= 1 && pieceValue <= 0xC)) {
        return EMPTY;
    }
    if (pieceValue < 7) {
      return WHITE;
    } else {
      return BLACK;
    }
}

// Converts a piece type and colour into its int value
int generate_piece_int(char pieceType, char color) {
  int num = 0;
  if (color == 'B') {
    num += 6;
  }
  switch(pieceType) {
    case 'P': num += 1; break;
    case 'N': num += 2; break;
    case 'B': num += 3; break;
    case 'R': num += 4; break;
    case 'Q': num += 5; break;
    case 'K': num += 6; break;
  }
  return num;
}

// Spawns a piece at a given coordinate
void set_piece(int piece, int row, int col, int** board) {
    board [row - 1][ROWS - col] = piece;
}

// Moves piece which exists at a given coordinate to another given coordinate
void move_piece(int row1, int col1, int row2, int col2, int** board) {
    int piece = get_piece(row1, col1, board);
    set_piece(0, row1, col1, board);
    set_piece(piece, row2, col2, board);
}

// Gets the piece at a particular coordinate
int get_piece(int row, int col, int** board) {
    return board[row - 1][ROWS - col];
}

// Prints the game board to stdout
void print_board(int** board) {
    // print top line
    for (int i = 0; i < (COLS * 2) + 1; i++) {
        printf("#");
    }
    printf("\n");
    for (int i = 0; i < ROWS; i++) {    // iterate through each row
        printf("#");    // print left-most line
        for (int j = 0; j < COLS; j++) {    // iterate through each col
            printf("%c", get_piece_type(board[j][i]));  // print the board piece
            printf("#");    // print line separator
        }
        printf("\n");
    }
    // print bottom line
    for (int i = 0; i < (COLS * 2) + 1; i++) {
        printf("#");
    }
    printf("\n");

    // Different Visual Style - Doesn't Look Good Yet
    // // print top line
    // for (int i = 0; i < (COLS * 6) + 1; i++) {
    //     printf("#");
    // }
    // printf("\n");
    // for (int i = 0; i < ROWS; i++) {    // iterate through each row
    //     printf("#");    // print left-most line
    //     for (int j = 0; j < COLS; j++) {    // iterate through each col
    //         printf("     ");  // print whitespace
    //         printf("#");    // print vertical line separator
    //     }
    //     printf("\n#"); // print left-most line on next row inside a grid-row
    //     for (int j = 0; j < COLS; j++) {    // iterate through each col
    //         printf("  %c  ", board[j][i]);  // print the board piece + whitespace
    //         printf("#");    // print vertical line separator
    //     }
    //     printf("\n#"); //print left-most line on next row inside a grid-row
    //     for (int j = 0; j < COLS; j++) {    // iterate through each col
    //         printf("     ");  // print the board piece
    //         printf("#");    // print vertical line separator
    //     }
    //     printf("\n");
    //     // print horizontal line separator
    //     for (int i = 0; i < (COLS * 6) + 1; i++) {
    //         printf("#");
    //     }
    //     printf("\n");
    // }
}

void error(int code) {
    switch (code) {
        case 1:
            fprintf(stderr, "Test Error\n");
            exit(1);
    }
}
