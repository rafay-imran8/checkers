// Checkers (American rules) - corrected and rewritten implementation
//
// Board coordinates: (row, col), row 0..7 top to bottom, col 0..7 left to right.
// Red starts on rows 5-7 (bottom) and moves toward row 0.
// White starts on rows 0-2 (top) and moves toward row 7.
//
// DESIGN NOTE (the one deliberate change from the original):
// The original asked the player to pick a *direction* ("forward-left or
// forward-right?") instead of a destination square. That makes it impossible
// to correctly support mandatory captures, multi-jumps, and king moves in all
// four diagonals without ambiguity, and was the source of most of the original
// bugs. This version asks for a FROM square and a TO square instead, which is
// the standard way to input a checkers move. Everything else (board layout,
// labels, single-file structure, Piece concept) is kept as close to the
// original as possible.

#include <iostream>
#include <vector>
#include <limits>
using namespace std;

enum class Player { RED, WHITE, NONE };

// Thrown when the input stream is exhausted, so the game can end
// gracefully instead of looping forever waiting for input that will
// never arrive.
struct EndOfInput {};

struct Piece
{
    Player player = Player::NONE;
    int row = -1, col = -1;
    bool isKing = false;
    bool alive = false;
};

struct Move
{
    int fromRow, fromCol;
    int toRow, toCol;
    bool isCapture;
    int capturedRow, capturedCol; // only valid if isCapture
};

class CheckersGame
{
public:
    CheckersGame()
    {
        board.assign(8, vector<char>(8, ' '));
        pieces.assign(24, Piece());
        currentPlayer = Player::RED;
        movesWithoutCaptureOrKing = 0;
        setupInitialPosition();
    }

    void run()
    {
        cout << "Welcome to Checkers!\n\n";
        printBoard();

        try
        {
            while (true)
            {
                if (!playerHasAnyLegalMove(currentPlayer))
                {
                    // Current player cannot move at all: they lose.
                    cout << (currentPlayer == Player::RED ? "Red" : "White")
                         << " has no legal moves. "
                         << (currentPlayer == Player::RED ? "White" : "Red")
                         << " wins!\n";
                    return;
                }

                playTurn();

                if (movesWithoutCaptureOrKing >= 80) // 40 moves per side with no progress
                {
                    cout << "\nNo captures or promotions in the last 40 moves per side. "
                            "The game is a draw!\n";
                    return;
                }

                currentPlayer = (currentPlayer == Player::RED ? Player::WHITE : Player::RED);
            }
        }
        catch (const EndOfInput &)
        {
            cout << "\nInput ended. Game stopped.\n";
        }
    }

private:
    vector<vector<char>> board;
    vector<Piece> pieces;
    Player currentPlayer;
    int movesWithoutCaptureOrKing;

    // ---------- setup / display ----------

    void setupInitialPosition()
    {
        int k = 0;
        for (int r = 0; r < 3; r++)
            for (int c = 0; c < 8; c++)
                if ((r + c) % 2 == 1)
                {
                    board[r][c] = 'W';
                    pieces[k] = {Player::WHITE, r, c, false, true};
                    k++;
                }
        for (int r = 5; r < 8; r++)
            for (int c = 0; c < 8; c++)
                if ((r + c) % 2 == 1)
                {
                    board[r][c] = 'R';
                    pieces[k] = {Player::RED, r, c, false, true};
                    k++;
                }
    }

    void printBoard() const
    {
        for (int r = 0; r < 8; r++)
        {
            cout << "  *---*---*---*---*---*---*---*---*\n";
            cout << r << " |";
            for (int c = 0; c < 8; c++)
            {
                char sym = board[r][c];
                bool king = false;
                if (sym != ' ')
                {
                    int idx = pieceIndexAt(r, c);
                    if (idx != -1 && pieces[idx].isKing)
                        king = true;
                }
                cout << ' ' << (king ? (sym == 'R' ? 'K' : 'Q') : sym) << " |";
            }
            cout << '\n';
        }
        cout << "  *---*---*---*---*---*---*---*---*\n";
        cout << "    0   1   2   3   4   5   6   7  \n";
        cout << "(K = Red king, Q = White king)\n\n";
    }

    // ---------- helpers ----------

    static bool onBoard(int r, int c) { return r >= 0 && r < 8 && c >= 0 && c < 8; }

    int pieceIndexAt(int r, int c) const
    {
        for (int i = 0; i < (int)pieces.size(); i++)
            if (pieces[i].alive && pieces[i].row == r && pieces[i].col == c)
                return i;
        return -1;
    }

    static char playerChar(Player p) { return p == Player::RED ? 'R' : 'W'; }
    static Player opponent(Player p) { return p == Player::RED ? Player::WHITE : Player::RED; }

    // Forward row-direction for a non-king piece of the given player.
    static int forwardDir(Player p) { return p == Player::RED ? -1 : 1; }

    // ---------- move generation ----------

    // All simple (non-capturing) single-step moves for one piece.
    vector<Move> simpleMovesFor(int idx) const
    {
        vector<Move> moves;
        const Piece &p = pieces[idx];
        vector<int> rowDirs;
        if (p.isKing)
            rowDirs = {-1, 1};
        else
            rowDirs = {forwardDir(p.player)};

        for (int dr : rowDirs)
            for (int dc : {-1, 1})
            {
                int nr = p.row + dr, nc = p.col + dc;
                if (onBoard(nr, nc) && board[nr][nc] == ' ')
                    moves.push_back({p.row, p.col, nr, nc, false, -1, -1});
            }
        return moves;
    }

    // All single-jump capture moves for one piece (from its current square).
    vector<Move> captureMovesFor(int idx) const
    {
        vector<Move> moves;
        const Piece &p = pieces[idx];
        vector<int> rowDirs;
        if (p.isKing)
            rowDirs = {-1, 1};
        else
            rowDirs = {forwardDir(p.player)};

        for (int dr : rowDirs)
            for (int dc : {-1, 1})
            {
                int mr = p.row + dr, mc = p.col + dc;         // square being jumped over
                int nr = p.row + 2 * dr, nc = p.col + 2 * dc; // landing square
                if (!onBoard(nr, nc) || !onBoard(mr, mc))
                    continue;
                if (board[mr][mc] == playerChar(opponent(p.player)) && board[nr][nc] == ' ')
                    moves.push_back({p.row, p.col, nr, nc, true, mr, mc});
            }
        return moves;
    }

    bool pieceHasCapture(int idx) const { return !captureMovesFor(idx).empty(); }

    bool playerHasAnyCapture(Player player) const
    {
        for (int i = 0; i < (int)pieces.size(); i++)
            if (pieces[i].alive && pieces[i].player == player && pieceHasCapture(i))
                return true;
        return false;
    }

    bool playerHasAnyLegalMove(Player player) const
    {
        for (int i = 0; i < (int)pieces.size(); i++)
        {
            if (!pieces[i].alive || pieces[i].player != player)
                continue;
            if (!captureMovesFor(i).empty() || !simpleMovesFor(i).empty())
                return true;
        }
        return false;
    }

    // Legal moves for a specific piece, taking the mandatory-capture rule into account.
    vector<Move> legalMovesFor(int idx, bool forcedCaptureActive) const
    {
        if (forcedCaptureActive)
            return captureMovesFor(idx);
        vector<Move> caps = captureMovesFor(idx);
        if (!caps.empty())
            return caps; // this piece must capture if it can
        return simpleMovesFor(idx);
    }

    // ---------- applying moves ----------

    void applyMove(int idx, const Move &m)
    {
        Piece &p = pieces[idx];
        board[p.row][p.col] = ' ';
        p.row = m.toRow;
        p.col = m.toCol;
        board[m.toRow][m.toCol] = playerChar(p.player);

        if (m.isCapture)
        {
            int capIdx = pieceIndexAt(m.capturedRow, m.capturedCol);
            if (capIdx != -1)
                pieces[capIdx].alive = false;
            board[m.capturedRow][m.capturedCol] = ' ';
            movesWithoutCaptureOrKing = 0;
        }
        else
        {
            movesWithoutCaptureOrKing++;
        }

        // Promotion happens after the move lands on the back rank.
        bool wasKing = p.isKing;
        if (p.player == Player::RED && p.row == 0)
            p.isKing = true;
        if (p.player == Player::WHITE && p.row == 7)
            p.isKing = true;
        if (!wasKing && p.isKing)
            movesWithoutCaptureOrKing = 0;
    }

    // ---------- input ----------

    // Returns false on bad (non-numeric) input that can be retried.
    // Sets eof = true if the input stream is exhausted, which callers
    // must check and handle by ending the game rather than looping forever.
    static bool readCoord(int &r, int &c, bool &eof)
    {
        eof = false;
        if (!(cin >> r >> c))
        {
            if (cin.eof())
            {
                eof = true;
                return false;
            }
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return false;
        }
        return true;
    }

    // ---------- turn logic ----------

    void playTurn()
    {
        Player me = currentPlayer;
        bool forced = playerHasAnyCapture(me);

        cout << (me == Player::RED ? "Red" : "White")
             << "'s turn.";
        if (forced)
            cout << " A capture is available and must be taken.";
        cout << '\n';

        int idx = choosePieceToMove(me, forced);

        // Perform this move, and continue capturing with the same piece
        // as long as further captures are available (multi-jump).
        bool isChain = false;
        while (true)
        {
            vector<Move> options = legalMovesFor(idx, isChain ? true : forced);
            Move chosen = chooseDestination(options);
            applyMove(idx, chosen);

            if (chosen.isCapture && pieceHasCapture(idx))
            {
                printBoard();
                cout << "Another capture is available with the same piece - you must continue jumping.\n";
                isChain = true;
                forced = true;
                continue;
            }
            break;
        }

        printBoard();
    }

    int choosePieceToMove(Player me, bool forced)
    {
        while (true)
        {
            cout << (me == Player::RED ? "Red" : "White")
                 << ", enter the row and column of the piece you want to move: ";
            int r, c;
            bool eof;
            if (!readCoord(r, c, eof))
            {
                if (eof)
                    throw EndOfInput{};
                cout << "Please enter two numbers.\n";
                continue;
            }
            if (!onBoard(r, c))
            {
                cout << "That square is off the board. Try again.\n";
                continue;
            }
            int idx = pieceIndexAt(r, c);
            if (idx == -1 || pieces[idx].player != me)
            {
                cout << "There is no piece of yours on that square. Try again.\n";
                continue;
            }
            if (forced && !pieceHasCapture(idx))
            {
                cout << "You must capture with a piece that has a capture available. Try again.\n";
                continue;
            }
            if (!forced && captureMovesFor(idx).empty() && simpleMovesFor(idx).empty())
            {
                cout << "That piece has no legal moves. Try again.\n";
                continue;
            }
            return idx;
        }
    }

    Move chooseDestination(const vector<Move> &options)
    {
        while (true)
        {
            cout << "Enter the destination row and column: ";
            int r, c;
            bool eof;
            if (!readCoord(r, c, eof))
            {
                if (eof)
                    throw EndOfInput{};
                cout << "Please enter two numbers.\n";
                continue;
            }
            for (const Move &m : options)
                if (m.toRow == r && m.toCol == c)
                    return m;
            cout << "That is not a legal destination for this piece. Try again.\n";
        }
    }
};

int main()
{
    CheckersGame game;
    game.run();
    return 0;
}