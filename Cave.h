#pragma once

#include <string>
#include <string_view>
#include <vector>
#include <utility>
#include <istream>

class Cave {
public:
    // Вспомогательные структуры
    //-------------------------------------------------------------
    enum class Resource { Iron, Gold, Gems, Exp };

    struct Room {
        int              number;
        std::vector<int> near_rooms;
        int iron = 0;
        int gold = 0;
        int gems = 0;
        int exp = 0;
    };

    struct Info {
        Room this_room;
        std::vector<std::pair<int, std::vector<int>>> near_rooms_data;
    };

    struct StartInfo {
        int      food;
        Resource double_resource;
    };
    //-------------------------------------------------------------
    
    // Публичный метод создания
    //-------------------------------------------------------------
    static Cave loadFromStream(std::istream& in);
    //-------------------------------------------------------------

    // Интерфейс взаимодействия
    //-------------------------------------------------------------
    Info      getRoomInfo(int roomNumber) const;
    StartInfo getStartInfo()             const;
    //-------------------------------------------------------------

private:
    //константы
    //-------------------------------------------------------------
    static constexpr int kMinN = 1;
    static constexpr int kMaxN = 255;
    static constexpr int kMinFood = 2;
    static constexpr int kMaxFood = 255;
    static constexpr int kMinResource = 0;
    static constexpr int kMaxResource = 255;
    //-------------------------------------------------------------

    // Допустимое количество токенов в строках комнат
    //-------------------------------------------------------------
    static constexpr int kTokensNoNeighborsNoRes = 1;
    static constexpr int kTokensNeighborsOnly = 2;
    static constexpr int kTokensResourcesOnly = 5;
    static constexpr int kTokensNeighborsAndRes = 6;
    //-------------------------------------------------------------

    // Поля класса
    //-------------------------------------------------------------
    int               size_;
    std::vector<Room> rooms_;
    int               food_;
    Resource          doubleResource_;
    //-------------------------------------------------------------

    // Приватный конструктор
    //-------------------------------------------------------------
    Cave(int size, std::vector<Room> rooms, int food, Resource doubleResource);
    Cave() = delete;
    //-------------------------------------------------------------
    // 
    // Вспомогательные функции парсинга
    //-------------------------------------------------------------
    static Resource           parseResource(std::string_view token, std::string_view line);
    static int                parseIntStrict(std::string_view token, std::string_view line);
    static int                parseResourceVal(std::string_view token, std::string_view line);
    static std::vector<std::string> tokenize(const std::string& line);

    static std::vector<int> parseNeighborToken(std::string_view token,
        int              maxRoom,
        std::string_view line);

    static Room parseRoomLine(const std::string& line, int maxRoom);
    //-------------------------------------------------------------
};