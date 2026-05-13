#include "Cave.h"
#include "ParseError.h"

#include <iostream>
#include <algorithm>
#include <climits>
#include <sstream>
#include <stdexcept>
#include <string>

// Конструктор
//-------------------------------------------------------------

Cave::Cave(int size, std::vector<Room> rooms, int food, Resource doubleResource)
    : size_(size), rooms_(std::move(rooms)), food_(food),
    doubleResource_(doubleResource) {
}
//-------------------------------------------------------------

// Вспомогательные функции парсинга
//-------------------------------------------------------------

Cave::Resource Cave::parseResource(std::string_view token,
    std::string_view line) {
    if (token == "iron")
        return Resource::Iron;
    if (token == "gold")
        return Resource::Gold;
    if (token == "gems")
        return Resource::Gems;
    if (token == "exp")
        return Resource::Exp;
    throw ParseError(std::string(line),
        "Unknown resource type: '" + std::string(token) + "'");
}

int Cave::parseIntStrict(std::string_view token, std::string_view line) {
    if (token.empty())
        throw ParseError(std::string(line), "Expected integer, got empty string");

    const bool negative = token[0] == '-';
    const auto digits = token.substr(negative ? 1 : 0);

    if (digits.empty())
        throw ParseError(std::string(line), "Bare minus sign");

    for (unsigned char ch : digits) {
        if (!std::isdigit(ch))
            throw ParseError(std::string(line),
                "Invalid integer: '" + std::string(token) + "'");
    }

    try {
        std::size_t consumed = 0;
        const long value = std::stol(std::string(token), &consumed);

        if (consumed != token.size() || value < INT_MIN || value > INT_MAX)
            throw ParseError(std::string(line),
                "Integer overflow: '" + std::string(token) + "'");

        return static_cast<int>(value);
    }
    catch (const ParseError&) {
        throw;
    }
    catch (...) {
        throw ParseError(std::string(line),
            "Invalid integer: '" + std::string(token) + "'");
    }
}

int Cave::parseResourceVal(std::string_view token, std::string_view line) {
    const int value = parseIntStrict(token, line);
    if (value < kMinResource || value > kMaxResource)
        throw ParseError(
            std::string(line),
            "Resource value out of range [" + std::to_string(kMinResource) + ", " +
            std::to_string(kMaxResource) + "]: '" + std::string(token) + "'");
    return value;
}

std::vector<std::string> Cave::tokenize(const std::string& line) {
    std::istringstream stream(line);
    std::vector<std::string> tokens;
    std::string token;
    while (stream >> token)
        tokens.push_back(std::move(token));
    return tokens;
}

std::vector<int> Cave::parseNeighborToken(std::string_view token, int maxRoom,
    std::string_view line) {
    for (unsigned char ch : token) {
        if (!std::isdigit(ch) && ch != ',')
            throw ParseError(std::string(line),
                "Invalid character '" +
                std::string(1, static_cast<char>(ch)) +
                "' in neighbor list: '" + std::string(token) + "'");
    }

    const bool isCommaSeparated = token.find(',') != std::string_view::npos;

    auto validateRoom = [&](int room) {
        if (room < 0 || room > maxRoom)
            throw ParseError(std::string(line), "Neighbor " + std::to_string(room) +
                " out of range [0, " +
                std::to_string(maxRoom) + "]");
        };

    if (!isCommaSeparated) {
        const int room = parseIntStrict(token, line);
        validateRoom(room);
        return { room };
    }

    if (token.front() == ',' || token.back() == ',')
        throw ParseError(std::string(line),
            "Neighbor list has leading or trailing comma");

    std::vector<int> neighbors;
    std::string tokenStr(token);

    size_t pos = 0;
    while (pos < tokenStr.size()) {
        size_t comma = tokenStr.find(',', pos);
        std::string part = (comma == std::string::npos)
            ? tokenStr.substr(pos)
            : tokenStr.substr(pos, comma - pos);

        if (part.empty())
            throw ParseError(std::string(line),
                "Empty element in neighbor list (double comma)");

        const int room = parseIntStrict(part, line);
        validateRoom(room);
        neighbors.push_back(room);

        if (comma == std::string::npos)
            break;
        pos = comma + 1;
    }

    auto sorted = neighbors;
    std::sort(sorted.begin(), sorted.end());
    if (std::adjacent_find(sorted.begin(), sorted.end()) != sorted.end())
        throw ParseError(std::string(line), "Duplicate neighbor in neighbor list");

    return neighbors;
}

Cave::Room Cave::parseRoomLine(const std::string& line, int maxRoom) {
    const auto tokens = tokenize(line);

    if (tokens.empty())
        throw ParseError(line, "Empty room line");

    const int number = parseIntStrict(tokens[0], line);
    if (number < 0 || number > maxRoom)
        throw ParseError(line, "Room number " + std::to_string(number) +
            " out of range [0, " + std::to_string(maxRoom) +
            "]");

    Room room;
    room.number = number;

    switch (static_cast<int>(tokens.size())) {
    case kTokensNoNeighborsNoRes:
        break;

    case kTokensNeighborsOnly:
        room.near_rooms = parseNeighborToken(tokens[1], maxRoom, line);
        break;

    case kTokensResourcesOnly:
        room.iron = parseResourceVal(tokens[1], line);
        room.gold = parseResourceVal(tokens[2], line);
        room.gems = parseResourceVal(tokens[3], line);
        room.exp = parseResourceVal(tokens[4], line);
        break;

    case kTokensNeighborsAndRes:
        room.near_rooms = parseNeighborToken(tokens[1], maxRoom, line);
        room.iron = parseResourceVal(tokens[2], line);
        room.gold = parseResourceVal(tokens[3], line);
        room.gems = parseResourceVal(tokens[4], line);
        room.exp = parseResourceVal(tokens[5], line);
        break;

    default:
        throw ParseError(line, "Room line must have 1, 2, 5, or 6 fields; got " +
            std::to_string(tokens.size()));
    }

    return room;
}
//-------------------------------------------------------------

// Публичный метод создания
//-------------------------------------------------------------

Cave Cave::loadFromStream(std::istream& in) {
    auto readLine = [&](std::string& line, std::string_view errorMsg) {
        if (!std::getline(in, line))
            throw ParseError("", std::string(errorMsg));
        };

    std::string line;
    readLine(line, "Empty file");

    const auto firstLineTokens = tokenize(line);
    if (firstLineTokens.size() != 1)
        throw ParseError(line, "First line must contain exactly one integer N");

    const int N = parseIntStrict(firstLineTokens[0], line);
    if (N < kMinN || N > kMaxN)
        throw ParseError(line, "N out of range [" + std::to_string(kMinN) + ", " +
            std::to_string(kMaxN) + "]");

    std::vector<Room> rooms(N + 1);
    std::vector<bool> seen(N + 1, false);

    for (int i = 0; i <= N; ++i) {
        readLine(line, "Unexpected end of file: missing room line");

        Room room = parseRoomLine(line, N);

        if (seen[room.number])
            throw ParseError(line,
                "Duplicate room number " + std::to_string(room.number));

        seen[room.number] = true;
        rooms[room.number] = std::move(room);
    }

    readLine(line, "Unexpected end of file: missing food/resource line");

    const auto lastTokens = tokenize(line);
    if (lastTokens.size() != 2)
        throw ParseError(
            line, "Last line must have exactly 2 fields (food resource); got " +
            std::to_string(lastTokens.size()));

    const int food = parseIntStrict(lastTokens[0], line);
    if (food < kMinFood || food > kMaxFood)
        throw ParseError(line, "Food out of range [" + std::to_string(kMinFood) +
            ", " + std::to_string(kMaxFood) + "]");

    const Resource doubled = parseResource(lastTokens[1], line);

    while (std::getline(in, line)) {
        if (!tokenize(line).empty())
            throw ParseError(line, "Extra data after last line");
    }

    return Cave(N, std::move(rooms), food, doubled);
}
//-------------------------------------------------------------

// Интерфейс взаимодействия
//-------------------------------------------------------------

Cave::Info Cave::getRoomInfo(int roomNumber) const {
    Info info;
    info.this_room = rooms_[roomNumber];

    for (int neighborNumber : info.this_room.near_rooms) {
        const Room& neighbor = rooms_[neighborNumber];
        info.near_rooms_data.push_back({ neighbor.number, neighbor.near_rooms });
    }

    return info;
}

Cave::StartInfo Cave::getStartInfo() const { return { food_, doubleResource_ }; }
//-------------------------------------------------------------