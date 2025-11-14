#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "players_data.h"   

#define TEAM_COUNT 10
#define MAX_NAME_LEN 50
#define ROLE_BATSMAN 1
#define ROLE_BOWLER 2
#define ROLE_ALLROUNDER 3

typedef struct PlayerNode {
    int playerId;
    char playerName[MAX_NAME_LEN + 1];
    char teamName[50];
    char roleName[20];
    int roleId;

    int totalRuns;
    float battingAverage;
    float strikeRate;
    int wickets;
    float economyRate;

    float performanceIndex;

    struct PlayerNode* next;
} PlayerNode;

typedef struct {
    int teamId;
    char teamName[50];
    int totalPlayers;
    float averageTeamStrikeRate;

    PlayerNode* head;
} Team;

/* ------------ FUNCTION PROTOTYPES ------------ */
float computePerformanceIndex(int roleId, float avg, float strikeRt, int wicketCount, float ecoRate);
void getRoleText(int roleId, char *roleText);

void insertPlayerIntoTeam(Team *team, PlayerNode *node);
int searchTeamById(Team teams[], int id);

void loadPlayerDataset(Team teams[]);
void computeTeamStrikeRate(Team *team);

void showTeamPlayers(Team *team);
void showTeamsSortedByStrikeRate(Team teams[]);
void showTopKPlayersInTeam(Team *team, int roleId, int K);
void showAllPlayersByRole(Team teams[], int roleId);

PlayerNode* findPlayerById(Team *team, int playerId);
void addPlayerWithValidation(Team *team);

/* Input helpers */
int readIntInRange(int min, int max, const char *prompt);
float readFloatMin(float min, const char *prompt);
void readValidName(char *outName);

float computePerformanceIndex(int roleId, float avg, float strikeRt, int wicketCount, float ecoRate) {
    if (roleId == ROLE_BATSMAN)
        return (avg * strikeRt) / 100.0f;

    if (roleId == ROLE_BOWLER)
        return (wicketCount * 2) + (100.0f - ecoRate);

    return ((avg * strikeRt) / 100.0f) + (wicketCount * 2);
}

void getRoleText(int roleId, char *roleText) {
    if (roleId == ROLE_BATSMAN) strcpy(roleText, "Batsman");
    else if (roleId == ROLE_BOWLER) strcpy(roleText, "Bowler");
    else strcpy(roleText, "All-rounder");
}

void insertPlayerIntoTeam(Team *team, PlayerNode *node) {
    node->next = team->head;
    team->head = node;
    team->totalPlayers++;
}

int searchTeamById(Team teams[], int id) {
    int low = 0, high = TEAM_COUNT - 1;

    while (low <= high) {
        int mid = (low + high) / 2;

        if (teams[mid].teamId == id) return mid;
        if (teams[mid].teamId < id) low = mid + 1;
        else high = mid - 1;
    }
    return -1;
}

PlayerNode* findPlayerById(Team *team, int playerId) {
    PlayerNode *node = team->head;
    while (node) {
        if (node->playerId == playerId) return node;
        node = node->next;
    }
    return NULL;
}

void loadPlayerDataset(Team teams[]) {
    int datasetSize = sizeof(players) / sizeof(players[0]);

    for (int i = 0; i < datasetSize; i++) {

        int teamIndex = -1;
        for (int teamLoopIndex = 0; teamLoopIndex < TEAM_COUNT; teamLoopIndex++) {
            if (strcmp(teams[teamLoopIndex].teamName, players[i].team) == 0) {
                teamIndex = teamLoopIndex;
                break;
            }
        }

        if (teamIndex == -1) continue;

        PlayerNode *newPlayer = (PlayerNode*)malloc(sizeof(PlayerNode));

        newPlayer->playerId = players[i].id;
        strncpy(newPlayer->playerName, players[i].name, MAX_NAME_LEN);
        strncpy(newPlayer->teamName, players[i].team, sizeof(newPlayer->teamName));
        strncpy(newPlayer->roleName, players[i].role, sizeof(newPlayer->roleName));

        if (strcmp(players[i].role, "Batsman") == 0) newPlayer->roleId = ROLE_BATSMAN;
        else if (strcmp(players[i].role, "Bowler") == 0) newPlayer->roleId = ROLE_BOWLER;
        else newPlayer->roleId = ROLE_ALLROUNDER;

        newPlayer->totalRuns = players[i].totalRuns;
        newPlayer->battingAverage = players[i].battingAverage;
        newPlayer->strikeRate = players[i].strikeRate;
        newPlayer->wickets = players[i].wickets;
        newPlayer->economyRate = players[i].economyRate;

        newPlayer->performanceIndex = computePerformanceIndex(
            newPlayer->roleId,
            newPlayer->battingAverage,
            newPlayer->strikeRate,
            newPlayer->wickets,
            newPlayer->economyRate
        );

        newPlayer->next = NULL;

        insertPlayerIntoTeam(&teams[teamIndex], newPlayer);
    }

    for (int i = 0; i < TEAM_COUNT; i++)
        computeTeamStrikeRate(&teams[i]);
}

void computeTeamStrikeRate(Team *team) {
    float totalStrikeRt = 0;
    int strikeRtCount = 0;

    PlayerNode *node = team->head;
    while (node) {
        if (node->roleId == ROLE_BATSMAN || node->roleId == ROLE_ALLROUNDER) {
            totalStrikeRt += node->strikeRate;
            strikeRtCount++;
        }
        node = node->next;
    }

    team->averageTeamStrikeRate = (strikeRtCount > 0 ? totalStrikeRt / strikeRtCount : 0);
}

void showTeamPlayers(Team *team) {
    printf("\nPlayers of %s (Team ID %d)\n", team->teamName, team->teamId);
    printf("--------------------------------------------------------------------\n");
    printf("ID    Name                     Role         Runs  Avg    Strikert    Wkts  Eco   PI\n");
    printf("--------------------------------------------------------------------\n");

    PlayerNode *node = team->head;
    while (node) {
        printf("%-5d %-25s %-12s %-6d %-6.1f %-6.1f %-6d %-5.1f %-6.2f\n",
               node->playerId, node->playerName, node->roleName,
               node->totalRuns, node->battingAverage, node->strikeRate,
               node->wickets, node->economyRate, node->performanceIndex);
        node = node->next;
    }

    printf("--------------------------------------------------------------------\n");
    printf("Total Players: %d\n", team->totalPlayers);
    printf("Average Team Strike Rate: %.2f\n", team->averageTeamStrikeRate);
}

void showTeamsSortedByStrikeRate(Team teams[]) {
    Team sortedTeams[TEAM_COUNT];
    memcpy(sortedTeams, teams, sizeof(sortedTeams));

    for (int i = 0; i < TEAM_COUNT; i++)
        for (int j = i + 1; j < TEAM_COUNT; j++)
            if (sortedTeams[i].averageTeamStrikeRate < sortedTeams[j].averageTeamStrikeRate) {
                Team tempTeam = sortedTeams[i];
                sortedTeams[i] = sortedTeams[j];
                sortedTeams[j] = tempTeam;
            }

    printf("\nTeams Sorted by Average Strike Rate:\n");
    printf("---------------------------------------------\n");
    printf("ID  Team               Avg SR    Players\n");
    printf("---------------------------------------------\n");

    for (int i = 0; i < TEAM_COUNT; i++) {
        printf("%-3d %-18s %-8.2f %-6d\n",
               sortedTeams[i].teamId,
               sortedTeams[i].teamName,
               sortedTeams[i].averageTeamStrikeRate,
               sortedTeams[i].totalPlayers);
    }
}

void showTopKPlayersInTeam(Team *team, int roleId, int K) {
    PlayerNode *playerList[200];
    int playerCount = 0;

    PlayerNode *node = team->head;
    while (node) {
        if (node->roleId == roleId)
            playerList[playerCount++] = node;
        node = node->next;
    }

    for (int i = 0; i < playerCount; i++)
        for (int j = i + 1; j < playerCount; j++)
            if (playerList[i]->performanceIndex < playerList[j]->performanceIndex) {
                PlayerNode *swapNode = playerList[i];
                playerList[i] = playerList[j];
                playerList[j] = swapNode;
            }

    char roleText[20];
    getRoleText(roleId, roleText);

    printf("\nTop %d %s(s) in %s:\n", K, roleText, team->teamName);
    printf("------------------------------------------\n");

    for (int i = 0; i < K && i < playerCount; i++)
        printf("%-4d %-25s PI: %.2f\n",
               playerList[i]->playerId,
               playerList[i]->playerName,
               playerList[i]->performanceIndex);
}

void showAllPlayersByRole(Team teams[], int roleId) {
    PlayerNode *playerList[1000];
    int playerCount = 0;

    for (int teamIndex = 0; teamIndex < TEAM_COUNT; teamIndex++) {
        PlayerNode *node = teams[teamIndex].head;
        while (node) {
            if (node->roleId == roleId)
                playerList[playerCount++] = node;
            node = node->next;
        }
    }

    for (int i = 0; i < playerCount; i++)
        for (int j = i + 1; j < playerCount; j++)
            if (playerList[i]->performanceIndex < playerList[j]->performanceIndex) {
                PlayerNode *swapNode = playerList[i];
                playerList[i] = playerList[j];
                playerList[j] = swapNode;
            }

    char roleText[20];
    getRoleText(roleId, roleText);

    printf("\nAll %s(s) Across All Teams:\n", roleText);
     printf(" ID         Name              Team        PI\n");
    printf("---------------------------------------------\n");
    for (int i = 0; i < playerCount; i++)
        printf("%-5d %-25s %-15s %.2f\n",
               playerList[i]->playerId,
               playerList[i]->playerName,
               playerList[i]->teamName,
               playerList[i]->performanceIndex);
}

int readIntInRange(int min, int max, const char *prompt) {
    char inputBuf[128];
    long inputValue;

    while (1) {
        printf("%s", prompt);
        fgets(inputBuf, sizeof(inputBuf), stdin);

        char extraChar;
        if (sscanf(inputBuf, "%ld %c", &inputValue, &extraChar) == 1) {
            if (inputValue >= min && inputValue <= max) return inputValue;
            printf("Error: Enter between %d and %d.\n", min, max);
        } else {
            printf("Error: Enter numbers only.\n");
        }
    }
}

float readFloatMin(float min, const char *prompt) {
    char inputBuf[128];
    double inputValue;

    while (1) {
        printf("%s", prompt);
        fgets(inputBuf, sizeof(inputBuf), stdin);

        char extraChar;
        if (sscanf(inputBuf, "%lf %c", &inputValue, &extraChar) == 1) {
            if (inputValue >= min) return inputValue;
            printf("Error: Must be >= %.2f.\n", min);
        } else {
            printf("Error: Enter numbers only.\n");
        }
    }
}

void readValidName(char *outName) {
    char inputBuf[200];

    while (1) {
        printf("Enter Name: ");
        fgets(inputBuf, sizeof(inputBuf), stdin);

        inputBuf[strcspn(inputBuf, "\n")] = 0;

        int valid = 1;
        for (int i = 0; inputBuf[i]; i++) {
            if (!isalpha(inputBuf[i]) && inputBuf[i] != ' ') {
                valid = 0;
                break;
            }
        }

        if (!valid || strlen(inputBuf) < 1 || strlen(inputBuf) > MAX_NAME_LEN) {
            printf("Invalid name. Use letters & spaces only.\n");
            continue;
        }

        strcpy(outName, inputBuf);
        return;
    }
}

void addPlayerWithValidation(Team *team) {
    if (team->totalPlayers >= 50) {
        printf("Team full (max 50).\n");
        return;
    }

    printf("\nAdd Player to %s\n", team->teamName);

    int playerId;
    while (1) {
        playerId = readIntInRange(1, 1000, "Enter Player ID: ");
        if (findPlayerById(team, playerId))
            printf("Player ID already exists. Try again.\n");
        else break;
    }

    PlayerNode *newPlayer = malloc(sizeof(PlayerNode));

    newPlayer->playerId = playerId;
    readValidName(newPlayer->playerName);

    int selectedRole = readIntInRange(1, 3, "Role (1-Batsman, 2-Bowler, 3-All-rounder): ");
    newPlayer->roleId = selectedRole;
    getRoleText(selectedRole, newPlayer->roleName);

    newPlayer->totalRuns = readIntInRange(0, 999999, "Total Runs: ");
    newPlayer->battingAverage = readFloatMin(0, "Batting Avg: ");
    newPlayer->strikeRate = readFloatMin(0, "Strike Rate: ");
    newPlayer->wickets = readIntInRange(0, 99999, "Wickets: ");
    newPlayer->economyRate = readFloatMin(0, "Economy Rate: ");

    strcpy(newPlayer->teamName, team->teamName);

    newPlayer->performanceIndex =
        computePerformanceIndex(newPlayer->roleId, newPlayer->battingAverage, newPlayer->strikeRate,
                                newPlayer->wickets, newPlayer->economyRate);

    newPlayer->next = NULL;

    insertPlayerIntoTeam(team, newPlayer);
    computeTeamStrikeRate(team);

    printf("Player added.\n");
}
/* ------------ MEMORY CLEANUP FUNCTIONS ------------ */
void freePlayerList(PlayerNode *head) {
    PlayerNode *temp;
    while (head != NULL) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

void freeAllMemory(Team teams[]) {
    for (int i = 0; i < TEAM_COUNT; i++) {
        freePlayerList(teams[i].head);
        teams[i].head = NULL;
        teams[i].totalPlayers = 0;
    }
}

int main() {

    Team teams[TEAM_COUNT] = {
        {1, "Afghanistan", 0, 0, NULL},
        {2, "Australia", 0, 0, NULL},
        {3, "Bangladesh", 0, 0, NULL},
        {4, "England", 0, 0, NULL},
        {5, "India", 0, 0, NULL},
        {6, "New Zealand", 0, 0, NULL},
        {7, "Pakistan", 0, 0, NULL},
        {8, "South Africa", 0, 0, NULL},
        {9, "Sri Lanka", 0, 0, NULL},
        {10, "West Indies", 0, 0, NULL},
    };

    loadPlayerDataset(teams);

    while (1) {
        printf("\n====== ICC ODI Player Analyzer ======\n");
        printf("1. Add Player\n");
        printf("2. Show Players of a Team\n");
        printf("3. Sort Teams by Avg Strike Rate\n");
        printf("4. Top K Players in a Team (By Role)\n");
        printf("5. All Players by Role Across Teams\n");
        printf("6. Exit\n");

        int choice = readIntInRange(1, 6, "Enter choice: ");

        switch (choice) {

            case 1: {
                int teamId = readIntInRange(1, 10, "Team ID (1–10): ");
                int teamIndex = searchTeamById(teams, teamId);
                addPlayerWithValidation(&teams[teamIndex]);
                break;
            }

            case 2: {
                int teamId = readIntInRange(1, 10, "Team ID (1–10): ");
                int teamIndex = searchTeamById(teams, teamId);
                showTeamPlayers(&teams[teamIndex]);
                break;
            }

            case 3:
                showTeamsSortedByStrikeRate(teams);
                break;

            case 4: {
                int teamId = readIntInRange(1, 10, "Team ID (1–10): ");
                int teamIndex = searchTeamById(teams, teamId);
                int roleId = readIntInRange(1, 3, "Role (1-Batsman,2-Bowler,3-All Rounder): ");
                int K = readIntInRange(1, 50, "Enter value of K: ");
                showTopKPlayersInTeam(&teams[teamIndex], roleId, K);
                break;
            }

            case 5: {
                int roleId = readIntInRange(1, 3, "Role (1-Batsman,2-Bowler,3-All Rounder): ");
                showAllPlayersByRole(teams, roleId);
                break;
            }

            case 6:
              printf("Freeing memory...\n");
              freeAllMemory(teams);
              printf("All memory freed. Exiting...\n");
              return 0;
        }
    }
}
