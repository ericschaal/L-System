//
// Created by eschaal on 3/24/17.
//

#include <iostream>
#include <regex>
#include <omp.h>
#include <assert.h>
#include <functional>

#define GEN_SIZE 12


using namespace std;
typedef vector<char>::iterator char_iterator;
typedef tuple<char_iterator, char_iterator> iterator_tuple;


class DfaResult;

DfaResult dfa(int state, vector<char>::iterator begin, vector<char>::iterator end, DfaResult& r);
void generateString(int length);
void printString(vector<char>* string);
void process(vector<iterator_tuple> tasks);
vector<iterator_tuple> divide(int threads);
void printTasks(vector<iterator_tuple> v);
vector<vector<char>> match(vector<char> s, char_iterator start, char_iterator end);
void deleteAll(char_iterator from, char_iterator to);

vector<char>::iterator lastFloat;
char generator[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'x'};

vector<char> *myString;
regex oneToNine = regex("[1-9]");
regex zeroToNine = regex("[0-9]");


class DfaResult {

public:
    int state;
    vector<function<void()>> functions;

    void addDelete(char_iterator from, char_iterator to) {
        function<void()> func = [from, to]() -> void {
            deleteAll(from, to);
        };
        functions.push_back(func);
    }

    void exec() {
        for (auto it = functions.begin(); it != functions.end(); ++it) {
            (*it)();
        }
    }

};

class Compound{

public:
    DfaResult results[5];

    void addResult(int index, DfaResult r) {
        results[index] = r;
        return;
    }


};


int main() {

    srand((unsigned)time(0));
    int threads = 1;
    int stringLength = 50;

    assert(stringLength > (threads+1));

    omp_set_num_threads(threads);
    char predef[] = {'7', '.', '4', '8', 'x', '0', '0', '.', '0', '5', 'x', '4', '0', '8', '3', '1', '8', 'x', '7', '9', '1', '0', '4', '8', '.', '6', '1' ,'4','.'};
    myString = new vector<char>(predef, predef+29);
    //generateString(stringLength);
    printString(myString);
    lastFloat = myString->begin();

    vector<iterator_tuple> v = divide(threads+1);

    process(v);
    printString(myString);
    //printTasks(v);

}


void printString(vector<char>* string) {
    for (auto it = string->begin(); it != string->end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}

void generateString(int length) {
    for (int i = 0; i <length; i++) {
        char c = generator[rand() % GEN_SIZE];
        myString->push_back(c);
    }
}


DfaResult dfa(int state, vector<char>::iterator begin, vector<char>::iterator end, DfaResult& r) {

    auto l = lastFloat;

    if (begin > myString->end()) {
        r.state = state;
        //r.addDelete(lastFloat, begin);
        return r;
    }

    if (begin > end) {
        r.state = state;
        return r;
    }

    switch (state) {
        case 0:
            if (regex_match(string(1,*begin), oneToNine))
                return dfa(2, begin+1, end, r);
            else if (*begin == '0')
                return dfa(1, begin+1, end, r);
            else {
                r.addDelete(lastFloat, begin);
                return dfa(0, begin+1, end, r);
                }
        case 1:
            if (*begin == '.')
                return dfa(3, begin+1, end, r);
            else {
                r.addDelete(lastFloat, begin);
                return dfa(0, begin, end, r);
            }
        case 2:
            if (regex_match(string(1,*begin), zeroToNine))
                return dfa(2, begin+1, end, r);
            else if (*begin == '.')
                return dfa(3, begin+1, end, r);
            else {
                r.addDelete(lastFloat, begin);
                return dfa(0, begin, end, r);
            }
        case 3:
            if (regex_match(string(1,*begin), zeroToNine))
                return dfa(4, begin+1, end, r);
            else {
                r.addDelete(lastFloat, begin);
                return dfa(0, begin, end, r);
            }
        case 4:
            lastFloat = begin;
            if (regex_match(string(1,*begin), zeroToNine)) {
                return dfa(4, begin + 1, end, r);
            }
            else {
                r.addDelete(lastFloat, begin);
                return dfa(0, begin + 1, end, r);
            }
        default:
            return DfaResult();
    }

}

void deleteAll(char_iterator from, char_iterator to) {
    for (auto it = from; it < to; ++it) {
        *it = ' ';
    }
}



void printTasks(vector<iterator_tuple> v) {

    for (vector<iterator_tuple>::iterator i = v.begin(); i != v.end(); ++i) {
        for (auto j = get<0>(*i); j != get<1>(*i); ++j) {
            cout << *j << " ";
        }
    }
    cout << endl;
}

vector<iterator_tuple> divide(int threads) {
    vector<iterator_tuple> v = vector<iterator_tuple>();
    int size = myString->size() / threads;
    for (int i = 0; i < threads-1; i++) {
        iterator_tuple t = tuple<char_iterator, char_iterator>(myString->begin()+size*i, myString->begin()+size*(i+1));
        v.push_back(t);
    }
    iterator_tuple lastTuple = tuple<char_iterator, char_iterator>(myString->begin()+size*(threads-1), myString->end());
    v.push_back(lastTuple);
    return v;
}



void process(vector<iterator_tuple> tasks) {
    DfaResult result;
    Compound compounds[tasks.size()-1];
    #pragma omp parallel
    {

        #pragma omp master
        {

            dfa(0, get<0>(*tasks.begin()), get<1>(*tasks.begin()), result);
            result.exec();

        }
        #pragma omp parallel for schedule(static, 1) collapse(1)
        for (int i = 1; i < tasks.size(); ++i) {
            {
                for (int j = 0; j < 5; j++) {
                    auto startIterator = get<0>(*(tasks.begin() + i));
                    auto endIterator = get<1>(*(tasks.begin() + i));
                    DfaResult r = DfaResult();
                    dfa(j, startIterator, endIterator, r);
                    compounds[i - 1].addResult(j, r);
                }
            }
        }

        #pragma omp barrier
    }

    int lastState = result.state;
    for (auto compound: compounds) {
        auto resultFromState = compound.results[lastState];
        resultFromState.exec();
        lastState = resultFromState.state;
    }


}