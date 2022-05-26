// OpenCVApplication.cpp : Defines the entry point for the console 
application.
//

#include "common.h"
#include <random>
#include <map>
#include <stack>
#include <queue>
#include <chrono>
#include <filesystem>
#include <cmath>

using namespace std;

map<char, string> pieces_path = {
        {'K', "pieces/BlackKing.png"},
        {'Q', "pieces/BlackQueen.png"},
        {'B', "pieces/BlackBishop.png"},
        {'N', "pieces/BlackKnight.png"},
        {'R', "pieces/BlackRook.png"},
        {'P', "pieces/BlackPawn.png"},
        {'k', "pieces/WhiteKing.png"},
        {'q', "pieces/WhiteQueen.png"},
        {'b', "pieces/WhiteBishop.png"},
        {'n', "pieces/WhiteKnight.png"},
        {'r', "pieces/WhiteRook.png"},
        {'p', "pieces/WhitePawn.png"}
};
const char NO_PIECE = ' ';
map<char, Mat> pieces_images = {};

int load_pieces()
{
    Size last_size;
    for (const auto& path: pieces_path) {
        Mat img = imread(path.second, CV_LOAD_IMAGE_UNCHANGED);
        if (last_size == Size()) {
            last_size = img.size();
            cout << "Piece size = " << last_size << endl;
        } else if (last_size != img.size()) {
            cerr << "Not all images have the same size!" << endl;
            return 1;
        }
        pieces_images[path.first] = img;
    }
    return 0;
}

bool is_transparent_color(const Mat& mat, int x, int y)
{
    if (mat.data[x * mat.step + y * 4 + 3] != 255)
        return true;
    return false;
}

bool piece_overlaps(const Mat& board, const Mat& piece, int start_x, int 
start_y)
{
    uchar *pb = board.data;
    uchar *pp = piece.data;
    for (int i = 0; i < piece.rows; i++) {
        for (int j = 0; j < piece.cols; j++) {
            if (!is_transparent_color(piece, i, j)) {
                for (int k = 0; k < 4; k++) {
                    if (pp[i * piece.step + 4 * j + k] != pb[(i + start_x) 
* board.step + 4 * (j + start_y) + k])
                        return false;
                }
            }
        }
    }
    return true;
}

Mat create_board(const vector<vector<char>>& board, const string& path = 
"output.png") {
    Size piece_size = pieces_images.begin()->second.size();
    int height = (int)board.size() * piece_size.height;
    int max_pieces = 0;
    for (const auto & row : board)
        if (row.size() > max_pieces)
            max_pieces = (int)row.size();

    int width = max_pieces * piece_size.width;
    Mat board_img = Mat::zeros(height, width, CV_8UC4);
    uchar *pb = board_img.data;
    for (int i = 0; i < board.size(); i++) {
        for (int j = 0; j < board[i].size(); j++) {
            if (board[i][j] != NO_PIECE) {
                auto image = pieces_images[board[i][j]];
                for (int y = 0; y < piece_size.height; y++) {
                    for (int x = 0; x < piece_size.width; x++) {
                        for (int k = 0; k < 4; k++) {
                            pb[(i * image.rows + y) * board_img.step + (j 
* image.cols + x) * 4 + k] =
                                    image.data[y * image.step + 4 * x + 
k];
                        }
                    }
                }
            }
        }
    }
    imwrite(path, board_img);
    return board_img;
}

vector<vector<char>> convert_image2board(const Mat& board_img) {
    vector<vector<char>> board;
   // bool found_any = false;
    Size piece_size = pieces_images.begin()->second.size();

    if (board_img.rows % piece_size.height != 0 || board_img.cols % 
piece_size.width) {
        cout << "The board is not a multiple of a piece size." << endl;
    } else {
        int rows = board_img.rows / piece_size.height;
        int cols = board_img.cols / piece_size.width;

        for (int i = 0; i < rows; i++) {
            board.emplace_back();
            for (int j = 0; j < cols; j++) {
                bool found = false;
                for (const auto &p: pieces_images) {
                    if (piece_overlaps(board_img, p.second,
                                       i * piece_size.height, j * 
piece_size.width)) {
                        found = true;
                        //found_any = true;
                        board[i].push_back(p.first);
                        break;
                    }
                }
                if (!found)
                    board[i].push_back(NO_PIECE);
            }
        }
    }
    /*if (!found_any) {
        cout << "No piece found on the board" << endl;
        board.clear();
    }*/
    return board;
}
void print_board(const vector<vector<char>>& board) {
    int rows = (int)board.size();
    int cols = 0;
    for (int i = 0; i < rows; i++) {
        cout << rows - i << " ";
        if (board[i].size() > cols)
            cols = (int)board[i].size();
        for (char j : board[i])
            cout << j;
        cout << "\n";
    }
    cout << "  ";
    for(int i = 0; i < cols; i++)
        cout << (char)('a' + i);
    cout << endl;
}
int main()
{
    int ret = load_pieces();
    if (ret)
        return ret;

    vector<vector<char>> test_board = {
            {'R', 'N', 'B', 'K', 'Q', 'B', 'N', 'R'},
            {'P', 'P', 'P', 'P', 'P', 'P', 'P', 'P'},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {' ', ' ', ' ', ' ', ' ', ' ', ' ', ' '},
            {'p', 'p', 'p', 'p', 'p', 'p', 'p', 'p'},
            {'r', 'n', 'b', 'q', 'k', 'b', 'n', 'r'}
    };

    Mat test_board_img = create_board(test_board, "test_image.png");
    imshow("Test board image", test_board_img);
    auto test_board_computed = convert_image2board(test_board_img);

    cout << "Original test board:" << endl;
    print_board(test_board);

    cout << "Computed test board:" << endl;
    print_board(test_board_computed);

    waitKey();
    char fname[MAX_PATH] = "";
    while (openFileDlg(fname))
    {
        auto begin = std::chrono::high_resolution_clock::now();
        Mat board_img = imread(fname, CV_LOAD_IMAGE_UNCHANGED);
        auto board = convert_image2board(board_img);
        print_board(board);
        auto end = std::chrono::high_resolution_clock::now();
        auto elapsed = 
std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin);
        printf("Processed in %.6f seconds.\n", elapsed.count() * 1e-9);
        imshow("Board image", board_img);
        waitKey();
    }
    return 0;
}

