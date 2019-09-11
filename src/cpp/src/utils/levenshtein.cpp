#pragma once

int levenshteinDistance(const vector <string> &s, const vector <string> &t) {
    int ls = s.size();
    int lt = t.size();
    vector<vector<int>> d(ls + 1, vector<int>(lt + 1));

    for (int i = 1; i <= ls; i++) {
        d.at(i).at(0) = i;
    }

    for (int j = 1; j <= lt; j++) {
        d.at(0).at(j) = j;
    }

    for (int j = 1; j <= lt; j++) {
        for (int i = 1; i <= ls; i++) {
            int cost = s.at(i - 1) == t.at(j - 1) ? 0 : 1;
            d.at(i).at(j) = min(
                {
                    d.at(i - 1).at(j) + 1,
                    d.at(i).at(j - 1) + 1,
                    d.at(i - 1).at(j - 1) + cost
                }
            );
        }
    }

    return d.at(ls).at(lt);
}
