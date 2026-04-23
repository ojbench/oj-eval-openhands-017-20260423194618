
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <string>
using namespace std;

// Constants
const int MAX_USERS = 10000;
const int MAX_TRAINS = 1000;
const int MAX_STATIONS = 100;
const int MAX_ORDERS = 100000;
const int MAX_STRING_LEN = 100;
const int MAX_USERNAME_LEN = 21;
const int MAX_PASSWORD_LEN = 31;
const int MAX_NAME_LEN = 16; // 5 Chinese characters * 3 + 1
const int MAX_EMAIL_LEN = 31;
const int MAX_TRAIN_ID_LEN = 21;
const int MAX_STATION_NAME_LEN = 31; // 10 Chinese characters * 3 + 1
const int MAX_DATE_LEN = 11; // mm-dd hh:mm format
const int MAX_TIME_LEN = 6; // hh:mm format
const int MAX_DAYS = 92; // June 1 to August 31, 2021

// User structure
struct User {
    char username[MAX_USERNAME_LEN];
    char password[MAX_PASSWORD_LEN];
    char name[MAX_NAME_LEN];
    char mailAddr[MAX_EMAIL_LEN];
    int privilege;
    bool isOnline;
    
    User() {
        memset(username, 0, sizeof(username));
        memset(password, 0, sizeof(password));
        memset(name, 0, sizeof(name));
        memset(mailAddr, 0, sizeof(mailAddr));
        privilege = 0;
        isOnline = false;
    }
};

// Seat availability structure for each train per day
struct TrainSeat {
    int seats[MAX_STATIONS - 1]; // seats available for each segment
    
    TrainSeat() {
        memset(seats, 0, sizeof(seats));
    }
};

// Train structure
struct Train {
    char trainID[MAX_TRAIN_ID_LEN];
    int stationNum;
    char stations[MAX_STATIONS][MAX_STATION_NAME_LEN];
    int seatNum;
    int prices[MAX_STATIONS - 1];
    char startTime[MAX_TIME_LEN];
    int travelTimes[MAX_STATIONS - 1];
    int stopoverTimes[MAX_STATIONS - 2];
    char saleDateStart[MAX_DATE_LEN];
    char saleDateEnd[MAX_DATE_LEN];
    char type;
    bool isReleased;
    TrainSeat dailySeats[MAX_DAYS]; // seat availability for each day
    
    Train() {
        memset(trainID, 0, sizeof(trainID));
        stationNum = 0;
        memset(stations, 0, sizeof(stations));
        seatNum = 0;
        memset(prices, 0, sizeof(prices));
        memset(startTime, 0, sizeof(startTime));
        memset(travelTimes, 0, sizeof(travelTimes));
        memset(stopoverTimes, 0, sizeof(stopoverTimes));
        memset(saleDateStart, 0, sizeof(saleDateStart));
        memset(saleDateEnd, 0, sizeof(saleDateEnd));
        type = ' ';
        isReleased = false;
        memset(dailySeats, 0, sizeof(dailySeats));
    }
};

// Order structure
struct Order {
    char username[MAX_USERNAME_LEN];
    char trainID[MAX_TRAIN_ID_LEN];
    char date[MAX_DATE_LEN];
    char fromStation[MAX_STATION_NAME_LEN];
    char toStation[MAX_STATION_NAME_LEN];
    int numTickets;
    int totalPrice;
    char status[20]; // success, pending, refunded
    long long timestamp;
    
    Order() {
        memset(username, 0, sizeof(username));
        memset(trainID, 0, sizeof(trainID));
        memset(date, 0, sizeof(date));
        memset(fromStation, 0, sizeof(fromStation));
        memset(toStation, 0, sizeof(toStation));
        numTickets = 0;
        totalPrice = 0;
        memset(status, 0, sizeof(status));
        timestamp = 0;
    }
};

// Global data structures
User users[MAX_USERS];
Train trains[MAX_TRAINS];
Order orders[MAX_ORDERS];
int userCount = 0;
int trainCount = 0;
int orderCount = 0;

// Hash table for users (open addressing)
struct UserHash {
    char username[MAX_USERNAME_LEN];
    int userIndex;
    bool isUsed;
    
    UserHash() {
        memset(username, 0, sizeof(username));
        userIndex = -1;
        isUsed = false;
    }
};

UserHash userHashTable[MAX_USERS * 2];

// Hash table for trains
struct TrainHash {
    char trainID[MAX_TRAIN_ID_LEN];
    int trainIndex;
    bool isUsed;
    
    TrainHash() {
        memset(trainID, 0, sizeof(trainID));
        trainIndex = -1;
        isUsed = false;
    }
};

TrainHash trainHashTable[MAX_TRAINS * 2];

// Simple hash function
unsigned int hashString(const char* str, int tableSize) {
    unsigned int hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash % tableSize;
}

// Find user index by username
int findUser(const char* username) {
    unsigned int hash = hashString(username, MAX_USERS * 2);
    unsigned int index = hash;
    
    while (userHashTable[index].isUsed) {
        if (strcmp(userHashTable[index].username, username) == 0) {
            return userHashTable[index].userIndex;
        }
        index = (index + 1) % (MAX_USERS * 2);
        if (index == hash) break;
    }
    return -1;
}

// Find train index by trainID
int findTrain(const char* trainID) {
    unsigned int hash = hashString(trainID, MAX_TRAINS * 2);
    unsigned int index = hash;
    
    while (trainHashTable[index].isUsed) {
        if (strcmp(trainHashTable[index].trainID, trainID) == 0) {
            return trainHashTable[index].trainIndex;
        }
        index = (index + 1) % (MAX_TRAINS * 2);
        if (index == hash) break;
    }
    return -1;
}

// Add user to hash table
bool addUserToHash(const char* username, int userIndex) {
    if (findUser(username) != -1) return false;
    
    unsigned int hash = hashString(username, MAX_USERS * 2);
    unsigned int index = hash;
    
    while (userHashTable[index].isUsed) {
        index = (index + 1) % (MAX_USERS * 2);
        if (index == hash) return false;
    }
    
    strcpy(userHashTable[index].username, username);
    userHashTable[index].userIndex = userIndex;
    userHashTable[index].isUsed = true;
    return true;
}

// Add train to hash table
bool addTrainToHash(const char* trainID, int trainIndex) {
    if (findTrain(trainID) != -1) return false;
    
    unsigned int hash = hashString(trainID, MAX_TRAINS * 2);
    unsigned int index = hash;
    
    while (trainHashTable[index].isUsed) {
        index = (index + 1) % (MAX_TRAINS * 2);
        if (index == hash) return false;
    }
    
    strcpy(trainHashTable[index].trainID, trainID);
    trainHashTable[index].trainIndex = trainIndex;
    trainHashTable[index].isUsed = true;
    return true;
}

// Parse command arguments
void parseCommand(const string& command, string& cmdName, string args[20], int& argCount) {
    cmdName = "";
    argCount = 0;
    
    size_t pos = 0;
    size_t end = command.find(' ');
    if (end == string::npos) {
        cmdName = command;
        return;
    }
    
    cmdName = command.substr(0, end);
    pos = end + 1;
    
    while (pos < command.length() && argCount < 20) {
        if (command[pos] == '-') {
            pos++;
            if (pos >= command.length()) break;
            
            char key = command[pos];
            pos++;
            
            while (pos < command.length() && command[pos] == ' ') pos++;
            if (pos >= command.length()) break;
            
            string value;
            if (command[pos] == '"') {
                pos++;
                while (pos < command.length() && command[pos] != '"') {
                    value += command[pos];
                    pos++;
                }
                pos++;
            } else {
                while (pos < command.length() && command[pos] != ' ') {
                    value += command[pos];
                    pos++;
                }
            }
            
            args[argCount] = string(1, key) + " " + value;
            argCount++;
        }
        
        while (pos < command.length() && command[pos] == ' ') pos++;
    }
}

// Get argument value by key
string getArgValue(const string args[20], int argCount, char key) {
    for (int i = 0; i < argCount; i++) {
        if (args[i].length() > 2 && args[i][0] == key && args[i][1] == ' ') {
            return args[i].substr(2);
        }
    }
    return "";
}

// Check if user is logged in
bool isUserLoggedIn(const char* username) {
    int userIndex = findUser(username);
    if (userIndex == -1) return false;
    return users[userIndex].isOnline;
}

// Function declarations
void clean();
void exit_program();

// Convert date string (mm-dd) to day index (0-91 for June 1 to August 31)
int dateToDayIndex(const char* date) {
    int month = (date[0] - '0') * 10 + (date[1] - '0');
    int day = (date[3] - '0') * 10 + (date[4] - '0');
    
    if (month == 6) return day - 1; // June 1 = 0
    if (month == 7) return 30 + day - 1; // July 1 = 30
    if (month == 8) return 61 + day - 1; // August 1 = 61
    return -1;
}

// Convert time string (hh:mm) to minutes
int timeToMinutes(const char* time) {
    int hour = (time[0] - '0') * 10 + (time[1] - '0');
    int minute = (time[3] - '0') * 10 + (time[4] - '0');
    return hour * 60 + minute;
}

// Format minutes to hh:mm string
void minutesToTime(int minutes, char* timeStr) {
    int hour = minutes / 60;
    int minute = minutes % 60;
    sprintf(timeStr, "%02d:%02d", hour % 24, minute);
}

// Add minutes to time and format as mm-dd hh:mm
void addMinutesToTime(const char* baseDate, const char* baseTime, int minutes, char* result) {
    int dayIndex = dateToDayIndex(baseDate);
    int baseMinutes = timeToMinutes(baseTime);
    int totalMinutes = dayIndex * 24 * 60 + baseMinutes + minutes;
    
    int newDayIndex = totalMinutes / (24 * 60);
    int newMinutes = totalMinutes % (24 * 60);
    
    // Convert back to date
    int month, day;
    if (newDayIndex < 30) {
        month = 6;
        day = newDayIndex + 1;
    } else if (newDayIndex < 61) {
        month = 7;
        day = newDayIndex - 30 + 1;
    } else {
        month = 8;
        day = newDayIndex - 61 + 1;
    }
    
    char timeStr[6];
    minutesToTime(newMinutes, timeStr);
    sprintf(result, "%02d-%02d %s", month, day, timeStr);
}

// Find station index in train
int findStationIndex(const Train& train, const char* stationName) {
    for (int i = 0; i < train.stationNum; i++) {
        if (strcmp(train.stations[i], stationName) == 0) {
            return i;
        }
    }
    return -1;
}

// Parse pipe-separated values
void parsePipeSeparated(const char* input, char result[][MAX_STRING_LEN], int maxCount, int& actualCount) {
    actualCount = 0;
    char temp[MAX_STRING_LEN];
    int tempIndex = 0;
    
    for (int i = 0; input[i] && actualCount < maxCount; i++) {
        if (input[i] == '|') {
            temp[tempIndex] = '\0';
            strcpy(result[actualCount], temp);
            actualCount++;
            tempIndex = 0;
        } else {
            temp[tempIndex++] = input[i];
        }
    }
    
    if (tempIndex > 0 && actualCount < maxCount) {
        temp[tempIndex] = '\0';
        strcpy(result[actualCount], temp);
        actualCount++;
    }
}

// Validate username format
bool isValidUsername(const char* username) {
    if (strlen(username) == 0 || strlen(username) > 20) return false;
    if (!(username[0] >= 'a' && username[0] <= 'z') && !(username[0] >= 'A' && username[0] <= 'Z')) {
        return false;
    }
    for (int i = 1; username[i]; i++) {
        char c = username[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
            return false;
        }
    }
    return true;
}

// Validate password format
bool isValidPassword(const char* password) {
    int len = strlen(password);
    if (len < 6 || len > 30) return false;
    for (int i = 0; password[i]; i++) {
        char c = password[i];
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_')) {
            return false;
        }
    }
    return true;
}

// add_user command
void add_user(const string args[20], int argCount) {
    string curUsername = getArgValue(args, argCount, 'c');
    string username = getArgValue(args, argCount, 'u');
    string password = getArgValue(args, argCount, 'p');
    string name = getArgValue(args, argCount, 'n');
    string mailAddr = getArgValue(args, argCount, 'm');
    string privilegeStr = getArgValue(args, argCount, 'g');
    
    // Check if this is the first user
    if (userCount == 0) {
        if (!isValidUsername(username.c_str()) || !isValidPassword(password.c_str()) || 
            name.empty() || mailAddr.empty()) {
            cout << "-1" << endl;
            return;
        }
        
        // Create first user with privilege 10
        if (userCount >= MAX_USERS) {
            cout << "-1" << endl;
            return;
        }
        
        strcpy(users[userCount].username, username.c_str());
        strcpy(users[userCount].password, password.c_str());
        strcpy(users[userCount].name, name.c_str());
        strcpy(users[userCount].mailAddr, mailAddr.c_str());
        users[userCount].privilege = 10;
        users[userCount].isOnline = false;
        
        if (!addUserToHash(username.c_str(), userCount)) {
            cout << "-1" << endl;
            return;
        }
        
        userCount++;
        cout << "0" << endl;
        return;
    }
    
    // Not first user - check permissions
    if (curUsername.empty() || username.empty() || password.empty() || 
        name.empty() || mailAddr.empty() || privilegeStr.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int curUserIndex = findUser(curUsername.c_str());
    if (curUserIndex == -1 || !users[curUserIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    int privilege = atoi(privilegeStr.c_str());
    if (privilege >= users[curUserIndex].privilege) {
        cout << "-1" << endl;
        return;
    }
    
    if (!isValidUsername(username.c_str()) || !isValidPassword(password.c_str()) ||
        findUser(username.c_str()) != -1) {
        cout << "-1" << endl;
        return;
    }
    
    if (userCount >= MAX_USERS) {
        cout << "-1" << endl;
        return;
    }
    
    strcpy(users[userCount].username, username.c_str());
    strcpy(users[userCount].password, password.c_str());
    strcpy(users[userCount].name, name.c_str());
    strcpy(users[userCount].mailAddr, mailAddr.c_str());
    users[userCount].privilege = privilege;
    users[userCount].isOnline = false;
    
    if (!addUserToHash(username.c_str(), userCount)) {
        cout << "-1" << endl;
        return;
    }
    
    userCount++;
    cout << "0" << endl;
}

// login command
void login(const string args[20], int argCount) {
    string username = getArgValue(args, argCount, 'u');
    string password = getArgValue(args, argCount, 'p');
    
    if (username.empty() || password.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int userIndex = findUser(username.c_str());
    if (userIndex == -1) {
        cout << "-1" << endl;
        return;
    }
    
    if (strcmp(users[userIndex].password, password.c_str()) != 0) {
        cout << "-1" << endl;
        return;
    }
    
    if (users[userIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    users[userIndex].isOnline = true;
    cout << "0" << endl;
}

// logout command
void logout(const string args[20], int argCount) {
    string username = getArgValue(args, argCount, 'u');
    
    if (username.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int userIndex = findUser(username.c_str());
    if (userIndex == -1 || !users[userIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    users[userIndex].isOnline = false;
    cout << "0" << endl;
}

// query_profile command
void query_profile(const string args[20], int argCount) {
    string curUsername = getArgValue(args, argCount, 'c');
    string username = getArgValue(args, argCount, 'u');
    
    if (curUsername.empty() || username.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int curUserIndex = findUser(curUsername.c_str());
    if (curUserIndex == -1 || !users[curUserIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    int userIndex = findUser(username.c_str());
    if (userIndex == -1) {
        cout << "-1" << endl;
        return;
    }
    
    // Check permission: same user or higher privilege
    if (curUserIndex != userIndex && users[curUserIndex].privilege <= users[userIndex].privilege) {
        cout << "-1" << endl;
        return;
    }
    
    cout << users[userIndex].username << " " << users[userIndex].name << " " 
         << users[userIndex].mailAddr << " " << users[userIndex].privilege << endl;
}

// modify_profile command
void modify_profile(const string args[20], int argCount) {
    string curUsername = getArgValue(args, argCount, 'c');
    string username = getArgValue(args, argCount, 'u');
    string password = getArgValue(args, argCount, 'p');
    string name = getArgValue(args, argCount, 'n');
    string mailAddr = getArgValue(args, argCount, 'm');
    string privilegeStr = getArgValue(args, argCount, 'g');
    
    if (curUsername.empty() || username.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int curUserIndex = findUser(curUsername.c_str());
    if (curUserIndex == -1 || !users[curUserIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    int userIndex = findUser(username.c_str());
    if (userIndex == -1) {
        cout << "-1" << endl;
        return;
    }
    
    // Check permission: same user or higher privilege
    if (curUserIndex != userIndex && users[curUserIndex].privilege <= users[userIndex].privilege) {
        cout << "-1" << endl;
        return;
    }
    
    // Check new privilege if provided
    if (!privilegeStr.empty()) {
        int newPrivilege = atoi(privilegeStr.c_str());
        if (newPrivilege >= users[curUserIndex].privilege) {
            cout << "-1" << endl;
            return;
        }
        users[userIndex].privilege = newPrivilege;
    }
    
    // Update other fields if provided
    if (!password.empty()) {
        if (!isValidPassword(password.c_str())) {
            cout << "-1" << endl;
            return;
        }
        strcpy(users[userIndex].password, password.c_str());
    }
    
    if (!name.empty()) {
        strcpy(users[userIndex].name, name.c_str());
    }
    
    if (!mailAddr.empty()) {
        strcpy(users[userIndex].mailAddr, mailAddr.c_str());
    }
    
    cout << users[userIndex].username << " " << users[userIndex].name << " " 
         << users[userIndex].mailAddr << " " << users[userIndex].privilege << endl;
}

// add_train command
void add_train(const string args[20], int argCount) {
    string trainID = getArgValue(args, argCount, 'i');
    string stationNumStr = getArgValue(args, argCount, 'n');
    string seatNumStr = getArgValue(args, argCount, 'm');
    string stationsStr = getArgValue(args, argCount, 's');
    string pricesStr = getArgValue(args, argCount, 'p');
    string startTime = getArgValue(args, argCount, 'x');
    string travelTimesStr = getArgValue(args, argCount, 't');
    string stopoverTimesStr = getArgValue(args, argCount, 'o');
    string saleDateStr = getArgValue(args, argCount, 'd');
    string type = getArgValue(args, argCount, 'y');
    
    if (trainID.empty() || stationNumStr.empty() || seatNumStr.empty() || 
        stationsStr.empty() || pricesStr.empty() || startTime.empty() || 
        travelTimesStr.empty() || stopoverTimesStr.empty() || saleDateStr.empty() || type.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    if (findTrain(trainID.c_str()) != -1) {
        cout << "-1" << endl;
        return;
    }
    
    if (trainCount >= MAX_TRAINS) {
        cout << "-1" << endl;
        return;
    }
    
    int stationNum = atoi(stationNumStr.c_str());
    int seatNum = atoi(seatNumStr.c_str());
    
    if (stationNum <= 1 || stationNum > MAX_STATIONS || seatNum <= 0 || seatNum > 100000) {
        cout << "-1" << endl;
        return;
    }
    
    // Parse stations
    char stations[MAX_STATIONS][MAX_STRING_LEN];
    int stationCount = 0;
    parsePipeSeparated(stationsStr.c_str(), stations, MAX_STATIONS, stationCount);
    if (stationCount != stationNum) {
        cout << "-1" << endl;
        return;
    }
    
    // Parse prices
    char prices[MAX_STATIONS][MAX_STRING_LEN];
    int priceCount = 0;
    parsePipeSeparated(pricesStr.c_str(), prices, MAX_STATIONS, priceCount);
    if (priceCount != stationNum - 1) {
        cout << "-1" << endl;
        return;
    }
    
    // Parse travel times
    char travelTimes[MAX_STATIONS][MAX_STRING_LEN];
    int travelTimeCount = 0;
    parsePipeSeparated(travelTimesStr.c_str(), travelTimes, MAX_STATIONS, travelTimeCount);
    if (travelTimeCount != stationNum - 1) {
        cout << "-1" << endl;
        return;
    }
    
    // Parse stopover times
    char stopoverTimes[MAX_STATIONS][MAX_STRING_LEN];
    int stopoverTimeCount = 0;
    parsePipeSeparated(stopoverTimesStr.c_str(), stopoverTimes, MAX_STATIONS, stopoverTimeCount);
    if (stopoverTimeCount != stationNum - 2) {
        cout << "-1" << endl;
        return;
    }
    
    // Parse sale dates
    char saleDates[2][MAX_STRING_LEN];
    int saleDateCount = 0;
    parsePipeSeparated(saleDateStr.c_str(), saleDates, 2, saleDateCount);
    if (saleDateCount != 2) {
        cout << "-1" << endl;
        return;
    }
    
    // Create train
    Train& train = trains[trainCount];
    strcpy(train.trainID, trainID.c_str());
    train.stationNum = stationNum;
    train.seatNum = seatNum;
    strcpy(train.startTime, startTime.c_str());
    train.type = type[0];
    strcpy(train.saleDateStart, saleDates[0]);
    strcpy(train.saleDateEnd, saleDates[1]);
    
    // Copy stations
    for (int i = 0; i < stationNum; i++) {
        strcpy(train.stations[i], stations[i]);
    }
    
    // Copy prices
    for (int i = 0; i < stationNum - 1; i++) {
        train.prices[i] = atoi(prices[i]);
    }
    
    // Copy travel times
    for (int i = 0; i < stationNum - 1; i++) {
        train.travelTimes[i] = atoi(travelTimes[i]);
    }
    
    // Copy stopover times
    for (int i = 0; i < stationNum - 2; i++) {
        train.stopoverTimes[i] = atoi(stopoverTimes[i]);
    }
    
    // Initialize seat availability for all days
    int startDay = dateToDayIndex(train.saleDateStart);
    int endDay = dateToDayIndex(train.saleDateEnd);
    if (startDay == -1 || endDay == -1 || startDay > endDay) {
        cout << "-1" << endl;
        return;
    }
    
    for (int day = startDay; day <= endDay; day++) {
        for (int i = 0; i < stationNum - 1; i++) {
            train.dailySeats[day].seats[i] = seatNum;
        }
    }
    
    if (!addTrainToHash(trainID.c_str(), trainCount)) {
        cout << "-1" << endl;
        return;
    }
    
    trainCount++;
    cout << "0" << endl;
}

// release_train command
void release_train(const string args[20], int argCount) {
    string trainID = getArgValue(args, argCount, 'i');
    
    if (trainID.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int trainIndex = findTrain(trainID.c_str());
    if (trainIndex == -1 || trains[trainIndex].isReleased) {
        cout << "-1" << endl;
        return;
    }
    
    trains[trainIndex].isReleased = true;
    cout << "0" << endl;
}

// query_train command
void query_train(const string args[20], int argCount) {
    string trainID = getArgValue(args, argCount, 'i');
    string date = getArgValue(args, argCount, 'd');
    
    if (trainID.empty() || date.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int trainIndex = findTrain(trainID.c_str());
    if (trainIndex == -1 || !trains[trainIndex].isReleased) {
        cout << "-1" << endl;
        return;
    }
    
    Train& train = trains[trainIndex];
    int dayIndex = dateToDayIndex(date.c_str());
    int startDay = dateToDayIndex(train.saleDateStart);
    int endDay = dateToDayIndex(train.saleDateEnd);
    
    if (dayIndex < startDay || dayIndex > endDay) {
        cout << "-1" << endl;
        return;
    }
    
    cout << train.trainID << " " << train.type << endl;
    
    int currentTime = timeToMinutes(train.startTime);
    int currentDay = dayIndex;
    
    for (int i = 0; i < train.stationNum; i++) {
        char arrivingTime[20], leavingTime[20];
        
        if (i == 0) {
            strcpy(arrivingTime, "xx-xx xx:xx");
            sprintf(leavingTime, "%s %s", date.c_str(), train.startTime);
        } else if (i == train.stationNum - 1) {
            addMinutesToTime(date.c_str(), train.startTime, currentTime - timeToMinutes(train.startTime), arrivingTime);
            strcpy(leavingTime, "xx-xx xx:xx");
        } else {
            addMinutesToTime(date.c_str(), train.startTime, currentTime - timeToMinutes(train.startTime), arrivingTime);
            currentTime += train.stopoverTimes[i - 1];
            addMinutesToTime(date.c_str(), train.startTime, currentTime - timeToMinutes(train.startTime), leavingTime);
        }
        
        int cumulativePrice = 0;
        for (int j = 0; j < i; j++) {
            cumulativePrice += train.prices[j];
        }
        
        int availableSeats;
        if (i == train.stationNum - 1) {
            strcpy(leavingTime, "xx-xx xx:xx");
            availableSeats = -1; // 'x'
        } else {
            availableSeats = train.dailySeats[dayIndex].seats[i];
        }
        
        cout << train.stations[i] << " " << arrivingTime << " -> " << leavingTime << " " 
             << cumulativePrice << " ";
        
        if (availableSeats == -1) {
            cout << "x";
        } else {
            cout << availableSeats;
        }
        cout << endl;
        
        if (i < train.stationNum - 1) {
            currentTime += train.travelTimes[i];
        }
    }
}

// delete_train command
void delete_train(const string args[20], int argCount) {
    string trainID = getArgValue(args, argCount, 'i');
    
    if (trainID.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    int trainIndex = findTrain(trainID.c_str());
    if (trainIndex == -1 || trains[trainIndex].isReleased) {
        cout << "-1" << endl;
        return;
    }
    
    // Remove from hash table
    unsigned int hash = hashString(trainID.c_str(), MAX_TRAINS * 2);
    unsigned int index = hash;
    
    while (trainHashTable[index].isUsed) {
        if (strcmp(trainHashTable[index].trainID, trainID.c_str()) == 0) {
            trainHashTable[index].isUsed = false;
            break;
        }
        index = (index + 1) % (MAX_TRAINS * 2);
        if (index == hash) break;
    }
    
    // Remove train by shifting
    for (int i = trainIndex; i < trainCount - 1; i++) {
        trains[i] = trains[i + 1];
    }
    trainCount--;
    
    cout << "0" << endl;
}

// Structure to hold ticket query results
struct TicketResult {
    char trainID[MAX_TRAIN_ID_LEN];
    char fromStation[MAX_STATION_NAME_LEN];
    char toStation[MAX_STATION_NAME_LEN];
    char leavingTime[20];
    char arrivingTime[20];
    int price;
    int seats;
    int travelTime;
    
    TicketResult() {
        memset(trainID, 0, sizeof(trainID));
        memset(fromStation, 0, sizeof(fromStation));
        memset(toStation, 0, sizeof(toStation));
        memset(leavingTime, 0, sizeof(leavingTime));
        memset(arrivingTime, 0, sizeof(arrivingTime));
        price = 0;
        seats = 0;
        travelTime = 0;
    }
};

// Calculate travel time between stations for a specific train
int calculateTravelTime(const Train& train, int fromIndex, int toIndex, const char* queryDate) {
    int totalTime = 0;
    int currentTime = timeToMinutes(train.startTime);
    
    // Calculate time to from station
    for (int i = 0; i < fromIndex; i++) {
        currentTime += train.travelTimes[i];
        if (i < fromIndex - 1) {
            currentTime += train.stopoverTimes[i];
        }
    }
    
    // Calculate time from from station to to station
    for (int i = fromIndex; i < toIndex; i++) {
        totalTime += train.travelTimes[i];
        if (i < toIndex - 1) {
            totalTime += train.stopoverTimes[i];
        }
    }
    
    return totalTime;
}

// Get minimum available seats between two stations
int getMinAvailableSeats(const Train& train, int dayIndex, int fromIndex, int toIndex) {
    int minSeats = train.seatNum;
    for (int i = fromIndex; i < toIndex; i++) {
        if (train.dailySeats[dayIndex].seats[i] < minSeats) {
            minSeats = train.dailySeats[dayIndex].seats[i];
        }
    }
    return minSeats;
}

// query_ticket command
void query_ticket(const string args[20], int argCount) {
    string fromStation = getArgValue(args, argCount, 's');
    string toStation = getArgValue(args, argCount, 't');
    string date = getArgValue(args, argCount, 'd');
    string sortType = getArgValue(args, argCount, 'p');
    
    if (fromStation.empty() || toStation.empty() || date.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    if (sortType.empty()) {
        sortType = "time";
    }
    
    TicketResult results[100];
    int resultCount = 0;
    
    // Search all trains
    for (int i = 0; i < trainCount; i++) {
        if (!trains[i].isReleased) continue;
        
        Train& train = trains[i];
        int fromIndex = findStationIndex(train, fromStation.c_str());
        int toIndex = findStationIndex(train, toStation.c_str());
        
        if (fromIndex == -1 || toIndex == -1 || fromIndex >= toIndex) continue;
        
        // Check if train operates on query date
        int queryDayIndex = dateToDayIndex(date.c_str());
        int startDay = dateToDayIndex(train.saleDateStart);
        int endDay = dateToDayIndex(train.saleDateEnd);
        
        if (queryDayIndex < startDay || queryDayIndex > endDay) continue;
        
        // Calculate departure time from fromStation
        int departureTime = timeToMinutes(train.startTime);
        for (int j = 0; j < fromIndex; j++) {
            departureTime += train.travelTimes[j];
            if (j < fromIndex - 1) {
                departureTime += train.stopoverTimes[j];
            }
        }
        
        // Calculate arrival time at toStation
        int arrivalTime = departureTime;
        for (int j = fromIndex; j < toIndex; j++) {
            arrivalTime += train.travelTimes[j];
            if (j < toIndex - 1) {
                arrivalTime += train.stopoverTimes[j];
            }
        }
        
        // Calculate price
        int price = 0;
        for (int j = fromIndex; j < toIndex; j++) {
            price += train.prices[j];
        }
        
        // Get available seats
        int availableSeats = getMinAvailableSeats(train, queryDayIndex, fromIndex, toIndex);
        
        if (availableSeats <= 0) continue;
        
        // Add result
        TicketResult& result = results[resultCount++];
        strcpy(result.trainID, train.trainID);
        strcpy(result.fromStation, fromStation.c_str());
        strcpy(result.toStation, toStation.c_str());
        
        char tempTime[20];
        addMinutesToTime(date.c_str(), train.startTime, departureTime - timeToMinutes(train.startTime), tempTime);
        strcpy(result.leavingTime, tempTime);
        
        addMinutesToTime(date.c_str(), train.startTime, arrivalTime - timeToMinutes(train.startTime), tempTime);
        strcpy(result.arrivingTime, tempTime);
        
        result.price = price;
        result.seats = availableSeats;
        result.travelTime = arrivalTime - departureTime;
    }
    
    // Sort results
    for (int i = 0; i < resultCount - 1; i++) {
        for (int j = i + 1; j < resultCount; j++) {
            bool shouldSwap = false;
            if (sortType == "time") {
                shouldSwap = results[i].travelTime > results[j].travelTime;
            } else {
                shouldSwap = results[i].price > results[j].price;
            }
            
            if (shouldSwap) {
                TicketResult temp = results[i];
                results[i] = results[j];
                results[j] = temp;
            }
        }
    }
    
    // Output results
    cout << resultCount << endl;
    for (int i = 0; i < resultCount; i++) {
        cout << results[i].trainID << " " << results[i].fromStation << " " 
             << results[i].leavingTime << " -> " << results[i].toStation << " " 
             << results[i].arrivingTime << " " << results[i].price << " " 
             << results[i].seats << endl;
    }
}

// query_transfer command
void query_transfer(const string args[20], int argCount) {
    string fromStation = getArgValue(args, argCount, 's');
    string toStation = getArgValue(args, argCount, 't');
    string date = getArgValue(args, argCount, 'd');
    string sortType = getArgValue(args, argCount, 'p');
    
    if (fromStation.empty() || toStation.empty() || date.empty()) {
        cout << "0" << endl;
        return;
    }
    
    if (sortType.empty()) {
        sortType = "time";
    }
    
    TicketResult bestFirst, bestSecond;
    bool found = false;
    int bestValue = (sortType == "time") ? 999999 : 999999999;
    
    // Try all possible transfer stations
    for (int transferTrainIdx = 0; transferTrainIdx < trainCount; transferTrainIdx++) {
        if (!trains[transferTrainIdx].isReleased) continue;
        
        Train& transferTrain = trains[transferTrainIdx];
        int transferFromIndex = findStationIndex(transferTrain, fromStation.c_str());
        
        if (transferFromIndex == -1) continue;
        
        // Check if transfer train operates on query date
        int queryDayIndex = dateToDayIndex(date.c_str());
        int startDay = dateToDayIndex(transferTrain.saleDateStart);
        int endDay = dateToDayIndex(transferTrain.saleDateEnd);
        
        if (queryDayIndex < startDay || queryDayIndex > endDay) continue;
        
        // Try all stations on this train as transfer points
        for (int stationIdx = transferFromIndex + 1; stationIdx < transferTrain.stationNum; stationIdx++) {
            char* transferStation = transferTrain.stations[stationIdx];
            
            // Find second train from transfer station to destination
            for (int secondTrainIdx = 0; secondTrainIdx < trainCount; secondTrainIdx++) {
                if (secondTrainIdx == transferTrainIdx || !trains[secondTrainIdx].isReleased) continue;
                
                Train& secondTrain = trains[secondTrainIdx];
                int secondFromIndex = findStationIndex(secondTrain, transferStation);
                int secondToIndex = findStationIndex(secondTrain, toStation.c_str());
                
                if (secondFromIndex == -1 || secondToIndex == -1 || secondFromIndex >= secondToIndex) continue;
                
                // Check if second train operates on appropriate date
                int secondQueryDayIndex = queryDayIndex;
                
                // Calculate arrival time at transfer station
                int firstDepartureTime = timeToMinutes(transferTrain.startTime);
                for (int j = 0; j < transferFromIndex; j++) {
                    firstDepartureTime += transferTrain.travelTimes[j];
                    if (j < transferFromIndex - 1) {
                        firstDepartureTime += transferTrain.stopoverTimes[j];
                    }
                }
                
                int firstArrivalTime = firstDepartureTime;
                for (int j = transferFromIndex; j < stationIdx; j++) {
                    firstArrivalTime += transferTrain.travelTimes[j];
                    if (j < stationIdx - 1) {
                        firstArrivalTime += transferTrain.stopoverTimes[j];
                    }
                }
                
                // Calculate second train departure time
                int secondDepartureTime = timeToMinutes(secondTrain.startTime);
                for (int j = 0; j < secondFromIndex; j++) {
                    secondDepartureTime += secondTrain.travelTimes[j];
                    if (j < secondFromIndex - 1) {
                        secondDepartureTime += secondTrain.stopoverTimes[j];
                    }
                }
                
                // Check if second train operates on the right day
                int secondStartDay = dateToDayIndex(secondTrain.saleDateStart);
                int secondEndDay = dateToDayIndex(secondTrain.saleDateEnd);
                
                if (secondQueryDayIndex < secondStartDay || secondQueryDayIndex > secondEndDay) continue;
                
                // Calculate total metrics
                int secondArrivalTime = secondDepartureTime;
                for (int j = secondFromIndex; j < secondToIndex; j++) {
                    secondArrivalTime += secondTrain.travelTimes[j];
                    if (j < secondToIndex - 1) {
                        secondArrivalTime += secondTrain.stopoverTimes[j];
                    }
                }
                
                int totalTime = (firstArrivalTime - firstDepartureTime) + (secondArrivalTime - secondDepartureTime);
                
                int firstPrice = 0;
                for (int j = transferFromIndex; j < stationIdx; j++) {
                    firstPrice += transferTrain.prices[j];
                }
                
                int secondPrice = 0;
                for (int j = secondFromIndex; j < secondToIndex; j++) {
                    secondPrice += secondTrain.prices[j];
                }
                
                int totalPrice = firstPrice + secondPrice;
                
                // Check seat availability
                int firstSeats = getMinAvailableSeats(transferTrain, queryDayIndex, transferFromIndex, stationIdx);
                int secondSeats = getMinAvailableSeats(secondTrain, secondQueryDayIndex, secondFromIndex, secondToIndex);
                
                if (firstSeats <= 0 || secondSeats <= 0) continue;
                
                // Check if this is better
                int currentValue = (sortType == "time") ? totalTime : totalPrice;
                if (!found || currentValue < bestValue) {
                    found = true;
                    bestValue = currentValue;
                    
                    // First train result
                    strcpy(bestFirst.trainID, transferTrain.trainID);
                    strcpy(bestFirst.fromStation, fromStation.c_str());
                    strcpy(bestFirst.toStation, transferStation);
                    
                    char tempTime[20];
                    addMinutesToTime(date.c_str(), transferTrain.startTime, firstDepartureTime - timeToMinutes(transferTrain.startTime), tempTime);
                    strcpy(bestFirst.leavingTime, tempTime);
                    
                    addMinutesToTime(date.c_str(), transferTrain.startTime, firstArrivalTime - timeToMinutes(transferTrain.startTime), tempTime);
                    strcpy(bestFirst.arrivingTime, tempTime);
                    
                    bestFirst.price = firstPrice;
                    bestFirst.seats = firstSeats;
                    
                    // Second train result
                    strcpy(bestSecond.trainID, secondTrain.trainID);
                    strcpy(bestSecond.fromStation, transferStation);
                    strcpy(bestSecond.toStation, toStation.c_str());
                    
                    addMinutesToTime(date.c_str(), secondTrain.startTime, secondDepartureTime - timeToMinutes(secondTrain.startTime), tempTime);
                    strcpy(bestSecond.leavingTime, tempTime);
                    
                    addMinutesToTime(date.c_str(), secondTrain.startTime, secondArrivalTime - timeToMinutes(secondTrain.startTime), tempTime);
                    strcpy(bestSecond.arrivingTime, tempTime);
                    
                    bestSecond.price = secondPrice;
                    bestSecond.seats = secondSeats;
                }
            }
        }
    }
    
    if (!found) {
        cout << "0" << endl;
    } else {
        cout << bestFirst.trainID << " " << bestFirst.fromStation << " " 
             << bestFirst.leavingTime << " -> " << bestFirst.toStation << " " 
             << bestFirst.arrivingTime << " " << bestFirst.price << " " 
             << bestFirst.seats << endl;
        
        cout << bestSecond.trainID << " " << bestSecond.fromStation << " " 
             << bestSecond.leavingTime << " -> " << bestSecond.toStation << " " 
             << bestSecond.arrivingTime << " " << bestSecond.price << " " 
             << bestSecond.seats << endl;
    }
}

// Get current timestamp (simplified)
long long getCurrentTimestamp() {
    static long long counter = 1000000;
    return counter++;
}

// buy_ticket command
void buy_ticket(const string args[20], int argCount) {
    string username = getArgValue(args, argCount, 'u');
    string trainID = getArgValue(args, argCount, 'i');
    string date = getArgValue(args, argCount, 'd');
    string numStr = getArgValue(args, argCount, 'n');
    string fromStation = getArgValue(args, argCount, 'f');
    string toStation = getArgValue(args, argCount, 't');
    string queueStr = getArgValue(args, argCount, 'q');
    
    if (username.empty() || trainID.empty() || date.empty() || numStr.empty() || 
        fromStation.empty() || toStation.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    if (queueStr.empty()) {
        queueStr = "false";
    }
    
    // Check if user is logged in
    int userIndex = findUser(username.c_str());
    if (userIndex == -1 || !users[userIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    // Find train
    int trainIndex = findTrain(trainID.c_str());
    if (trainIndex == -1 || !trains[trainIndex].isReleased) {
        cout << "-1" << endl;
        return;
    }
    
    Train& train = trains[trainIndex];
    int fromIndex = findStationIndex(train, fromStation.c_str());
    int toIndex = findStationIndex(train, toStation.c_str());
    
    if (fromIndex == -1 || toIndex == -1 || fromIndex >= toIndex) {
        cout << "-1" << endl;
        return;
    }
    
    // Check if train operates on query date
    int queryDayIndex = dateToDayIndex(date.c_str());
    int startDay = dateToDayIndex(train.saleDateStart);
    int endDay = dateToDayIndex(train.saleDateEnd);
    
    if (queryDayIndex < startDay || queryDayIndex > endDay) {
        cout << "-1" << endl;
        return;
    }
    
    int numTickets = atoi(numStr.c_str());
    if (numTickets <= 0) {
        cout << "-1" << endl;
        return;
    }
    
    // Check seat availability
    int availableSeats = getMinAvailableSeats(train, queryDayIndex, fromIndex, toIndex);
    
    // Calculate price
    int totalPrice = 0;
    for (int i = fromIndex; i < toIndex; i++) {
        totalPrice += train.prices[i];
    }
    totalPrice *= numTickets;
    
    if (orderCount >= MAX_ORDERS) {
        cout << "-1" << endl;
        return;
    }
    
    // Create order
    Order& order = orders[orderCount];
    strcpy(order.username, username.c_str());
    strcpy(order.trainID, trainID.c_str());
    strcpy(order.date, date.c_str());
    strcpy(order.fromStation, fromStation.c_str());
    strcpy(order.toStation, toStation.c_str());
    order.numTickets = numTickets;
    order.totalPrice = totalPrice;
    order.timestamp = getCurrentTimestamp();
    
    if (availableSeats >= numTickets) {
        // Purchase successful
        strcpy(order.status, "success");
        
        // Update seat availability
        for (int i = fromIndex; i < toIndex; i++) {
            train.dailySeats[queryDayIndex].seats[i] -= numTickets;
        }
        
        orderCount++;
        cout << totalPrice << endl;
    } else if (queueStr == "true") {
        // Add to queue
        strcpy(order.status, "pending");
        orderCount++;
        cout << "queue" << endl;
    } else {
        cout << "-1" << endl;
    }
}

// query_order command
void query_order(const string args[20], int argCount) {
    string username = getArgValue(args, argCount, 'u');
    
    if (username.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    // Check if user is logged in
    int userIndex = findUser(username.c_str());
    if (userIndex == -1 || !users[userIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    // Count user's orders
    int userOrderCount = 0;
    int userOrderIndices[MAX_ORDERS];
    
    for (int i = 0; i < orderCount; i++) {
        if (strcmp(orders[i].username, username.c_str()) == 0) {
            userOrderIndices[userOrderCount++] = i;
        }
    }
    
    // Sort by timestamp (newest first)
    for (int i = 0; i < userOrderCount - 1; i++) {
        for (int j = i + 1; j < userOrderCount; j++) {
            if (orders[userOrderIndices[i]].timestamp < orders[userOrderIndices[j]].timestamp) {
                int temp = userOrderIndices[i];
                userOrderIndices[i] = userOrderIndices[j];
                userOrderIndices[j] = temp;
            }
        }
    }
    
    cout << userOrderCount << endl;
    
    for (int i = 0; i < userOrderCount; i++) {
        Order& order = orders[userOrderIndices[i]];
        
        // Find train to get times
        int trainIndex = findTrain(order.trainID);
        if (trainIndex == -1) continue;
        
        Train& train = trains[trainIndex];
        int fromIndex = findStationIndex(train, order.fromStation);
        int toIndex = findStationIndex(train, order.toStation);
        
        if (fromIndex == -1 || toIndex == -1) continue;
        
        // Calculate departure and arrival times
        int departureTime = timeToMinutes(train.startTime);
        for (int j = 0; j < fromIndex; j++) {
            departureTime += train.travelTimes[j];
            if (j < fromIndex - 1) {
                departureTime += train.stopoverTimes[j];
            }
        }
        
        int arrivalTime = departureTime;
        for (int j = fromIndex; j < toIndex; j++) {
            arrivalTime += train.travelTimes[j];
            if (j < toIndex - 1) {
                arrivalTime += train.stopoverTimes[j];
            }
        }
        
        char leavingTime[20], arrivingTime[20];
        addMinutesToTime(order.date, train.startTime, departureTime - timeToMinutes(train.startTime), leavingTime);
        addMinutesToTime(order.date, train.startTime, arrivalTime - timeToMinutes(train.startTime), arrivingTime);
        
        cout << "[" << order.status << "] " << order.trainID << " " << order.fromStation << " " 
             << leavingTime << " -> " << order.toStation << " " << arrivingTime << " " 
             << (order.totalPrice / order.numTickets) << " " << order.numTickets << endl;
    }
}

// refund_ticket command
void refund_ticket(const string args[20], int argCount) {
    string username = getArgValue(args, argCount, 'u');
    string nStr = getArgValue(args, argCount, 'n');
    
    if (username.empty()) {
        cout << "-1" << endl;
        return;
    }
    
    if (nStr.empty()) {
        nStr = "1";
    }
    
    // Check if user is logged in
    int userIndex = findUser(username.c_str());
    if (userIndex == -1 || !users[userIndex].isOnline) {
        cout << "-1" << endl;
        return;
    }
    
    int orderNum = atoi(nStr.c_str());
    if (orderNum <= 0) {
        cout << "-1" << endl;
        return;
    }
    
    // Get user's orders sorted by timestamp (newest first)
    int userOrderCount = 0;
    int userOrderIndices[MAX_ORDERS];
    
    for (int i = 0; i < orderCount; i++) {
        if (strcmp(orders[i].username, username.c_str()) == 0) {
            userOrderIndices[userOrderCount++] = i;
        }
    }
    
    // Sort by timestamp (newest first)
    for (int i = 0; i < userOrderCount - 1; i++) {
        for (int j = i + 1; j < userOrderCount; j++) {
            if (orders[userOrderIndices[i]].timestamp < orders[userOrderIndices[j]].timestamp) {
                int temp = userOrderIndices[i];
                userOrderIndices[i] = userOrderIndices[j];
                userOrderIndices[j] = temp;
            }
        }
    }
    
    if (orderNum > userOrderCount) {
        cout << "-1" << endl;
        return;
    }
    
    int orderIndex = userOrderIndices[orderNum - 1];
    Order& order = orders[orderIndex];
    
    if (strcmp(order.status, "refunded") == 0) {
        cout << "-1" << endl;
        return;
    }
    
    if (strcmp(order.status, "success") == 0) {
        // Return seats
        int trainIndex = findTrain(order.trainID);
        if (trainIndex != -1) {
            Train& train = trains[trainIndex];
            int fromIndex = findStationIndex(train, order.fromStation);
            int toIndex = findStationIndex(train, order.toStation);
            
            if (fromIndex != -1 && toIndex != -1) {
                int dayIndex = dateToDayIndex(order.date);
                if (dayIndex >= 0 && dayIndex < MAX_DAYS) {
                    for (int i = fromIndex; i < toIndex; i++) {
                        train.dailySeats[dayIndex].seats[i] += order.numTickets;
                    }
                }
            }
        }
    }
    
    strcpy(order.status, "refunded");
    cout << "0" << endl;
}

// Main command processing function
void processCommand(const string& command) {
    string cmdName;
    string args[20];
    int argCount = 0;
    
    parseCommand(command, cmdName, args, argCount);
    
    if (cmdName == "add_user") {
        add_user(args, argCount);
    } else if (cmdName == "login") {
        login(args, argCount);
    } else if (cmdName == "logout") {
        logout(args, argCount);
    } else if (cmdName == "query_profile") {
        query_profile(args, argCount);
    } else if (cmdName == "modify_profile") {
        modify_profile(args, argCount);
    } else if (cmdName == "add_train") {
        add_train(args, argCount);
    } else if (cmdName == "release_train") {
        release_train(args, argCount);
    } else if (cmdName == "query_train") {
        query_train(args, argCount);
    } else if (cmdName == "delete_train") {
        delete_train(args, argCount);
    } else if (cmdName == "query_ticket") {
        query_ticket(args, argCount);
    } else if (cmdName == "query_transfer") {
        query_transfer(args, argCount);
    } else if (cmdName == "buy_ticket") {
        buy_ticket(args, argCount);
    } else if (cmdName == "query_order") {
        query_order(args, argCount);
    } else if (cmdName == "refund_ticket") {
        refund_ticket(args, argCount);
    } else if (cmdName == "clean") {
        clean();
    } else if (cmdName == "exit") {
        exit_program();
    }
}

// clean command
void clean() {
    // Clear all data
    memset(users, 0, sizeof(users));
    memset(trains, 0, sizeof(trains));
    memset(orders, 0, sizeof(orders));
    memset(userHashTable, 0, sizeof(userHashTable));
    memset(trainHashTable, 0, sizeof(trainHashTable));
    
    userCount = 0;
    trainCount = 0;
    orderCount = 0;
    
    cout << "0" << endl;
}

// exit command
void exit_program() {
    // Log out all users
    for (int i = 0; i < userCount; i++) {
        users[i].isOnline = false;
    }
    
    cout << "bye" << endl;
    exit(0);
}

int main() {
    // Initialize data structures
    memset(users, 0, sizeof(users));
    memset(trains, 0, sizeof(trains));
    memset(orders, 0, sizeof(orders));
    memset(userHashTable, 0, sizeof(userHashTable));
    memset(trainHashTable, 0, sizeof(trainHashTable));
    
    userCount = 0;
    trainCount = 0;
    orderCount = 0;
    
    string command;
    while (getline(cin, command)) {
        if (command.empty()) continue;
        processCommand(command);
    }
    
    return 0;
}
