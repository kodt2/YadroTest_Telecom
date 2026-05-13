#include <algorithm>
#include <iostream>
#include <climits>
#include <queue>
#include <sstream>

#include "Bot.h"

Bot::Bot(const Cave &cave) : cave(cave), startInfo(cave.getStartInfo()) {
  pos = 0;
  food = startInfo.food;
  budget = startInfo.food / 2;
  visit(pos);
}

void Bot::simulate(std::ostream &out) {

  // фаза исследования
  //--------------------------------
  while (budget > 0) {
    int target = findNextTarget();
    if (target == -1)
      break;
    std::vector<int> path = shortestPath(pos, target, false);
    if (path.empty())
      break;

    for (size_t i = 1; i < path.size(); ++i) {
      if (budget <= 0)
        break;
      int nextRoom = path[i];
      out << "go " << nextRoom << "\n";
      budget--;
      food--;
      pos = nextRoom;

      if (visited.find(nextRoom) == visited.end()) {
        visit(nextRoom);
        printState(nextRoom, out);
        collectFirst(nextRoom, out);
      } else {
        if (nextRoom != 0)
          printState(nextRoom, out);
      }
    }
  }
  //--------------------------------

  // Фаза возвращения
  //--------------------------------
  if (pos == 0) {
    out << "result " << totalCollected[0] << " " << totalCollected[1] << " "
        << totalCollected[2] << " " << totalCollected[3] << " " << totalValue
        << "\n";
    return;
  }

  std::vector<int> path = shortestPath(pos, 0, true);
  int distance = static_cast<int>(path.size()) - 1;
  int extraFood = food - distance;

  for (size_t i = 0; i < path.size(); ++i) {
    int curRoom = path[i];
    if (curRoom == 0) {
      if (pos != 0) {
        out << "go 0\n";
        food--;
        pos = 0;
      }
      break;
    }
    while (extraFood > 0) {
      if (!collectExtra(curRoom, out))
        break;
      extraFood--;
    }
    if (i + 1 < path.size()) {
      int nextRoom = path[i + 1];
      out << "go " << nextRoom << "\n";
      food--;
      pos = nextRoom;
      if (nextRoom != 0)
        printState(nextRoom, out);
    }
  }
  //--------------------------------
  out << "result " << totalCollected[0] << " " << totalCollected[1] << " "
      << totalCollected[2] << " " << totalCollected[3] << " " << totalValue
      << "\n";
}

int Bot::valueOf(Cave::Resource res) const {
  int base = 0;
  switch (res) {
  case Cave::Resource::Iron:
    base = 7;
    break;
  case Cave::Resource::Gold:
    base = 11;
    break;
  case Cave::Resource::Gems:
    base = 23;
    break;
  case Cave::Resource::Exp:
    base = 1;
    break;
  }
  if (res == startInfo.double_resource)
    base *= 2;
  return base;
}

void Bot::visit(int room) {
  visited.insert(room);
  Cave::Info info = cave.getRoomInfo(room);
  addRoomInfo(room, info);
  resources[room] = {info.this_room.iron, info.this_room.gold,
                     info.this_room.gems, info.this_room.exp};
  collectedFlags[room].reset();
}

void Bot::addRoomInfo(int room, const Cave::Info &info) {
  for (const auto &p : info.near_rooms_data) {
    int neighbor = p.first;
    graph[room].push_back(neighbor);
    graph[neighbor].push_back(room);

    for (int nn : p.second) {
      if (nn != room) {
        graph[neighbor].push_back(nn);
        graph[nn].push_back(neighbor);
        if (graph.find(nn) == graph.end()) {
          graph[nn] = {};
        }
      }
    }
  }

  for (auto &kv : graph) {
    auto &v = kv.second;
    std::sort(v.begin(), v.end());
    v.erase(std::unique(v.begin(), v.end()), v.end());
  }
}

void Bot::collectFirst(int room, std::ostream &out) {
  auto &res = resources[room];
  int bestIdx = -1;
  int bestVal = -1;
  for (int i = 0; i < 4; ++i) {
    if (res[i] > 0) {
      int v = valueOf(static_cast<Cave::Resource>(i));
      if (v > bestVal) {
        bestVal = v;
        bestIdx = i;
      }
    }
  }
  if (bestIdx == -1)
    return;

  int amount = res[bestIdx];
  res[bestIdx] = 0;
  totalCollected[bestIdx] += amount;
  totalValue += amount * bestVal;
  collectedFlags[room].set(bestIdx);

  out << "collect ";
  switch (bestIdx) {
  case 0:
    out << "iron";
    break;
  case 1:
    out << "gold";
    break;
  case 2:
    out << "gems";
    break;
  case 3:
    out << "exp";
    break;
  }
  out << "\n";
  printState(room, out);
}

bool Bot::collectExtra(int room, std::ostream &out) {
  auto &res = resources[room];
  int bestIdx = -1;
  int bestVal = -1;
  for (int i = 0; i < 4; ++i) {
    if (res[i] > 0) {
      int v = valueOf(static_cast<Cave::Resource>(i));
      if (v > bestVal) {
        bestVal = v;
        bestIdx = i;
      }
    }
  }
  if (bestIdx == -1)
    return false;

  int amount = res[bestIdx];
  res[bestIdx] = 0;
  totalCollected[bestIdx] += amount;
  totalValue += amount * bestVal;
  collectedFlags[room].set(bestIdx);

  food--;

  out << "collect ";
  switch (bestIdx) {
  case 0:
    out << "iron";
    break;
  case 1:
    out << "gold";
    break;
  case 2:
    out << "gems";
    break;
  case 3:
    out << "exp";
    break;
  }
  out << "\n";
  printState(room, out);
  return true;
}

int Bot::findNextTarget() const {
  auto it = graph.find(pos);
  if (it != graph.end()) {
    for (int nb : it->second) {
      if (visited.find(nb) == visited.end())
        return nb;
    }
  }
  std::map<int, int> dist;
  std::queue<int> q;
  dist[pos] = 0;
  q.push(pos);
  int bestTarget = -1, bestDist = INT_MAX;
  while (!q.empty()) {
    int v = q.front();
    q.pop();
    auto itv = graph.find(v);
    if (itv == graph.end())
      continue;
    for (int nb : itv->second) {
      if (dist.find(nb) == dist.end()) {
        dist[nb] = dist[v] + 1;
        q.push(nb);
        if (visited.find(nb) == visited.end()) {
          if (dist[nb] < bestDist ||
              (dist[nb] == bestDist && nb < bestTarget)) {
            bestDist = dist[nb];
            bestTarget = nb;
          }
        }
      }
    }
  }
  return bestTarget;
}

std::vector<int> Bot::shortestPath(int from, int to, bool onlyVisited) const {
  std::map<int, int> parent;
  std::queue<int> q;
  q.push(from);
  parent[from] = -1;
  while (!q.empty()) {
    int v = q.front();
    q.pop();
    if (v == to)
      break;
    auto it = graph.find(v);
    if (it == graph.end())
      continue;
    std::vector<int> nb_sorted = it->second;
    std::sort(nb_sorted.begin(), nb_sorted.end());
    for (int nb : nb_sorted) {
      if (parent.find(nb) == parent.end()) {
        if (onlyVisited && visited.find(nb) == visited.end())
          continue;
        parent[nb] = v;
        q.push(nb);
      }
    }
  }
  if (parent.find(to) == parent.end())
    return {};
  std::vector<int> path;
  for (int cur = to; cur != -1; cur = parent[cur])
    path.push_back(cur);
  std::reverse(path.begin(), path.end());
  return path;
}

void Bot::printState(int room, std::ostream &out) const {
  out << "state " << room;
  const auto &res = resources.at(room);
  const auto &fl = collectedFlags.at(room);
  for (int i = 0; i < 4; ++i) {
    out << " ";
    if (fl[i])
      out << "_";
    else
      out << res[i];
  }
  out << "\n";
}