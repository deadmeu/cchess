struct Game {
    int** board;
    int rounds;
    char gameOver;
    int startX;
    int startY;
    int endX;
    int endY;
    char curPlayer;
};

typedef struct Game Game;

void handle_input(Game*);
int is_move_legal(int, int, int, int, Game*);
int is_piece_move_legal(int, int, int, int, int**);
int check_for_collision(int, int, int, int, int**);
int is_square_empty(int, int, int**);
void process_game(Game*);
void create_board(Game*);
void reset_board(int**);
int convert_hex_char_to_int(char);
char get_piece_type(int);
int get_piece_color(int);
int generate_piece_int(char, char);
void set_piece(int, int, int, int**);
void move_piece(int, int, int, int, int**);
int get_piece(int, int, int**);
void print_board(int**);
void error(int);
