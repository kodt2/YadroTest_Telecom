#pragma once

#include <vector>
#include <map>
#include <set>
#include <bitset>
#include <array>
#include <iosfwd>

#include "Cave.h"

class Bot {
public:
    //конструктор
    //--------------------------------
    explicit Bot(const Cave& cave);
    //--------------------------------

    //основная функция симуляции
    //--------------------------------
    void simulate(std::ostream& out);
    //--------------------------------

private:
    //поля класса
    //--------------------------------
    const Cave& cave;
    const Cave::StartInfo startInfo;

    int pos;
    int food;
    int budget;

    std::set<int> visited;
    std::map<int, std::vector<int>> graph;


    std::map<int, std::array<int, 4>> resources;
    std::map<int, std::bitset<4>> collectedFlags;

    int totalCollected[4] = { 0,0,0,0 };
    int totalValue = 0;
    //--------------------------------

    //функция рассчета ценности ресурса
    //--------------------------------
    int valueOf(Cave::Resource res) const;
    //--------------------------------

    //функция собирающая информацию при первом посещении
    //--------------------------------
    void visit(int room);
    //--------------------------------

    //функция обрабатывающая информацию при 1 посещении
    //--------------------------------
    void addRoomInfo(int room, const Cave::Info& info);
    //--------------------------------

    //функции сбора ресурсов
    //--------------------------------
    void collectFirst(int room, std::ostream& out);
    bool collectExtra(int room, std::ostream& out);
    //--------------------------------

    //функция поиска цели по правилам бота
    //--------------------------------
    int findNextTarget() const;
    //--------------------------------

    //функция поиска пути
    //--------------------------------
    std::vector<int> shortestPath(int from, int to, bool onlyVisited) const;
    //--------------------------------

    //функция вывода состояния
    //--------------------------------
    void printState(int room, std::ostream& out) const;
    //--------------------------------
};