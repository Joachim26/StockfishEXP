/*
  Stockfish, a UCI chess playing engine derived from Glaurung 2.1
  Copyright (C) 2004-2022 The Stockfish developers (see AUTHORS file)

  Stockfish is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Stockfish is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include <vector>

#include "misc.h"
#include "movepick.h"
#include "types.h"

namespace Stockfish {

class Position;

namespace Search {

/// Threshold used for countermoves based pruning
constexpr int CounterMovePruneThreshold = 0;


/// Stack struct keeps track of the information we need to remember from nodes
/// shallower and deeper in the tree during the search. Each search thread has
/// its own array of Stack objects, indexed by the current ply.

struct Stack {

  Move* pv;
  PieceToHistory* continuationHistory;
  int ply;
  Move currentMove;
  Move excludedMove;
  Move killers[2];
  Value staticEval;
  Depth depth;
  int statScore;
  int moveCount;
  bool inCheck;
  bool ttPv;
  bool ttHit;
  int doubleExtensions;
};


/// RootMove struct is used for moves at the root of the tree. For each root move
/// we store a score and a PV (really a refutation in the case of moves which
/// fail low). Score is normally set at -VALUE_INFINITE for all non-pv moves.

struct RootMove {

  explicit RootMove(Move m) : pv(1, m) {}
  bool extract_ponder_from_tt(Position& pos);
  bool operator==(const Move& m) const { return pv[0] == m; }
  bool operator<(const RootMove& m) const { // Sort in descending order
    return m.visits != visits ? m.visits < visits
           : m.score != score ? m.score < score
                              : m.previousScore < previousScore;
  }

  Value score = -VALUE_INFINITE;
  Value previousScore = -VALUE_INFINITE;
  Value averageScore = -VALUE_INFINITE;
  int selDepth = 0;
  int tbRank = 0;
  int visits = 0;
  Value tbScore;
  std::vector<Move> pv;
};

typedef std::vector<RootMove> RootMoves;


/// A small stack for the MCTS search

struct MctsStack {

  StateInfo st;
  int ply;
};


/// MctsNode holds all the info needed, like the place in the
/// HashTable, proof and disproof number, etc.

struct MctsNode {

  MctsNode(std::vector<MctsNode>::iterator idx, std::vector<MctsNode>::iterator pidx, Move m, bool exp, bool term, int , int ) :
    index(idx), parentIndex(pidx), move(m), isExpanded(exp), isTerminal(term), pn(v), dn(tr) {
  }

  std::vector<MctsNode>::iterator id()       const { return index; }
  std::vector<MctsNode>::iterator parentId() const { return parentIndex; }

  int PN() const { return pn; }
  int DN() const { return dn; }

  Move action()      const { return move; }
  bool is_expanded() const { return isExpanded; }
  bool is_terminal() const { return isTerminal; }

  void update_pn(int newPN) { pn = newPN; }
  void update_dn(int newDN) { dn = newDN; }

  void mark_as_expanded() { isExpanded = true; }
  void mark_as_terminal() { isTerminal = true; }

  std::vector<MctsNode>::iterator index;                 // Index of this node in the table
  std::vector<MctsNode>::iterator parentIndex;           // Index of the parent node
  Move move;                                             // Move which led to this position
  bool isExpanded;                                       // True if all child nodes have been generated
  bool isTerminal;                                       // Terminal node?
  int pn;                                       // Proof number
  int dn;                                    // Disproof number
  std::vector<std::vector<MctsNode>::iterator> children; // Holds the indices of all child nodes
  std::vector<Move> legalMoves;                          // All legal moves of this position
};

typedef std::vector<MctsNode> MctsHash;


/// LimitsType struct stores information sent by GUI about available time to
/// search the current move, maximum depth/time, or if we are in analysis mode.

struct LimitsType {

  LimitsType() { // Init explicitly due to broken value-initialization of non POD in MSVC
    time[WHITE] = time[BLACK] = inc[WHITE] = inc[BLACK] = npmsec = movetime = TimePoint(0);
    movestogo = depth = mate = perft = infinite = 0;
    nodes = 0;
  }

  bool use_time_management() const {
    return time[WHITE] || time[BLACK];
  }

  std::vector<Move> searchmoves;
  TimePoint time[COLOR_NB], inc[COLOR_NB], npmsec, movetime, startTime;
  int movestogo, depth, mate, perft, infinite;
  int64_t nodes;
};

extern LimitsType Limits;

void init();
void clear();

} // namespace Search

} // namespace Stockfish

#endif // #ifndef SEARCH_H_INCLUDED
