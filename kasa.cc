#include <iostream>
#include <regex>
#include <algorithm>
#include <string>
#include <climits>

using namespace std;

regex course("^(\\d+)( ([1-2]{0,1}\\d:[0-5]\\d) ([a-zA-Z_^]+))+$");
regex time_stop("(\\d{1,2}):([0-5]\\d) ([a-zA-Z_^]+)");
regex add_ticket("^([a-zA-Z ]+) (\\d+)\\.(\\d{2}) ([1-9]\\d*)$");
regex query("^\\? ([a-zA-Z_^]+)(( [\\d]+ [a-zA-Z_^]+)+)$");

int numberOfTickets = 0;
int numberOfLines = 0;

//W mapie trzymane są pary (nr kursu)
map<int, vector<pair<int, string>>> mapa;
map<string, pair<int, int>> tickets;
int lineCount = 0;

void addCourse(string, int);
void addTicket(string, smatch);
void answer(string);
void findTickets(int);


int main() {
    smatch sm;
    string input;
    while (getline(cin, input)) {
        ++lineCount;
        if (input.empty()) {
            //ignore empty line
        } else if (regex_match(input, sm, course)) {
            addCourse(input, stoi(sm[1]));
        } else if (regex_match(input, sm, add_ticket)) {
            addTicket(input, sm);
        } else if (regex_match(input, query)) {
            //todo usunąc śmieci
            /*cout << "\n Bilety: \n";
            for(auto it : tickets) {
                cout << it.first << " " << it.second.first << " " << it.second.second << endl;
            }
            cout << "\n Kursy: \n";
            for(auto it : mapa) {
                cout << it.first << " ";
                for (auto itt : it.second) {
                    cout << itt.first << " " << itt.second << " ";
                }
                cout << endl;
            }*/
            //todo tu koniec smieci
            answer(input);
        } else {
            cerr << "Error in line " << lineCount << ": " << input << endl;
        }
    }
    cout << numberOfTickets << "\n";
    return 0;
}

void addCourse(string input, int lineNumber) {
    smatch sm;
    int time, previousTime = 0;
    string stopName;

    if(mapa.find(lineNumber) != mapa.end()) {
        cerr << "Error in line " << lineCount << ": " << input << endl;
        return;
    }

    string pom = input;

    while (regex_search(pom, sm, time_stop)) {
        time = stoi(sm[1]) * 60 + stoi(sm[2]);
        stopName = sm[3];
        //todo czy czasy mogą być równe
        bool stop = false;
        for(auto it : mapa[lineNumber]) {
            if (it.second == stopName)
                stop = true;
        }
        if(time < 355 || time > 1281 || previousTime >= time || stop) {
            cerr << "Error in line " << lineCount << ": " << input << endl;
            mapa.erase(lineNumber);
            return;
        }
        previousTime = time;
        pom = sm.suffix().str();
        mapa[lineNumber].emplace_back(make_pair(time, stopName));
    }
}

void addTicket(string input, smatch sm) {
    string name = sm[1];
    if(tickets.find(name) != tickets.end()) {
        cerr << "Error in line " << lineCount << ": " << input << endl;
        return;
    }
    tickets[name] = make_pair(stoi(sm[3]) + 100 * stoi(sm[2]), stoi(sm[4]));
}

int dp[4][10000];

void findTickets(int allTime) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j <= 2 * allTime; j++) {
            dp[i][j] = INT_MAX / 2;
        }
    }
    dp[0][0] = 0;

    /*cout << allTime << ":\n";
    for (auto it : tickets) {
        cout << it.first << " " << it.second.first << " " << it.second.second << "\n";
    }*/

    for (int i = 1; i < 4; i++) {
        for (auto it : tickets) {
            for (int j = it.second.second; j <= 2 * allTime; j++) {
                dp[i][j] = min(dp[i][j], dp[i - 1][j - it.second.second] + it.second.first);
            }
        }
    }

    int minimum = INT_MAX / 2, minimumI = 0, minimumJ = 0;
    for (int i = 1; i < 4; i++) {
        for (int j = allTime + 1; j <= 2 * allTime; j++) {
            if (dp[i][j] < minimum) {
                minimum = dp[i][j];
                minimumI = i;
                minimumJ = j;
            }
        }
    }

    //cout << minimum << " " << minimumI << " " << minimumJ << "\n";

    int oneTicket = INT_MAX / 2;
    for (auto it : tickets) {
        if (it.second.second > allTime) {
            oneTicket = min(oneTicket, it.second.first);
        }
    }

    if (oneTicket < minimum) {
        for (auto it : tickets) {
            if (it.second.second > allTime && it.second.first == oneTicket) {
                cout << "! " << it.first << "\n";
                numberOfTickets++;
            }
        }
    }
    else if (minimum < INT_MAX / 2) {
        cout << "! ";
        bool isNext = false;
        while(minimumI != 0) {
            for (auto it : tickets) {
                if (dp[minimumI][minimumJ] == dp[minimumI - 1][minimumJ - it.second.second] + it.second.first) {
                    minimumI--;
                    minimumJ -= it.second.second;
                    if (isNext)
                        cout << "; ";
                    isNext = true;
                    cout << it.first;
                    numberOfTickets++;
                }
            }
        }
        cout << "\n";
    }
    else {
        cout << ":-|\n";
    }
}

void answer(string input) {

    //cout << input << "\n";

    smatch sm;
    regex_match(input, sm, query);

    /*for (auto it : sm) {
        cout << it << "\n";
    }*/

    stringstream strumien;

    int lineNumber, allTime = 0, currTime = 0;
    bool toWait = false, isPossible = true;
    string start = sm[1], next, whereWait;

    strumien.clear();
    strumien.str(sm[2]);

    //cout << sm[2] << "\n";

    while(strumien >> lineNumber >> next) {
        int t1 = 0, t2 = 0;
        //cout << start << " " << lineNumber << " " << next << "\n";
        if (mapa.find(lineNumber) == mapa.end()) {
            isPossible = false;
            break;
        }
        for (auto it : mapa[lineNumber]) {
            if (it.second == start) {
                if (toWait == false && currTime != 0 && currTime < it.first) {
                    toWait = true;
                    whereWait = start;
                }
                if (toWait == false && currTime != 0 && currTime > it.first) {
                    isPossible = false;
                }
                t1 = it.first;
            }
            if (it.second == next) {
                t2 = it.first;
            }
        }
        if (t1 == 0 || t2 == 0 || t1 > t2) {
            isPossible = false;
            break;
        }
        allTime += t2 - t1;
        currTime = t2;
        start = next;
    }

    if (isPossible == false)
        cerr << "Error in line " << lineCount << ": " << input << endl;
    else if (toWait)
        cout << ":-( " << whereWait << "\n";
    else
        findTickets(allTime);
}
