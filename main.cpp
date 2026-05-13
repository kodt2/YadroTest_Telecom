#include <algorithm>
#include <array>
#include <bitset>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "Bot.h"
#include "Cave.h"
#include "ParseError.h"

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "no input file\n";
    return 1;
  }
  std::ifstream instream(argv[1]);
  if (!instream.is_open()) {
    std::cerr << "error while reading file\n";
    return 1;
  }
  try {
    Cave cave = Cave::loadFromStream(instream);
    std::ofstream out("result.txt");
    if (!out.is_open()) {
      std::cerr << "cannot open result.txt\n";
      return 1;
    }
    Bot bot(cave);
    bot.simulate(out);
  } catch (const ParseError &e) {
    std::ofstream out("result.txt");
    if (out.is_open())
      out << e.error_line;
    std::cerr << e.what();
    return 1;
  }
  return 0;
}