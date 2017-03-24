//
// Created by eschaal on 3/24/17.
//

#include <iostream>
#include <regex>
#include <vector>
#define GEN_SIZE 12


using namespace std;
typedef vector<char>::iterator char_iterator;
typedef tuple<char_iterator, char_iterator> iterator_tuple;

bool dfa(int state, vector<char>::iterator it);
void generateString(int length);
void printString();
void process();
vector<iterator_tuple> divide(int threads);

char generator[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'x'};

vector<char> *myString;
regex oneToNine = regex("[1-9]");
regex zeroToNine = regex("[0-9]");




int main() {

    srand((unsigned)time(0));

    myString = new vector<char>();
    generateString(10);
    printString();
    vector<iterator_tuple> v = divide(20);
    //bool result = dfa(0, myString->begin());

//    for (vector<iterator_tuple>::iterator i = v.begin(); i != v.end(); ++i) {
//        for (auto j = get<0>(*i); j != get<1>(*i); ++j) {
//            cout << *j << " ";
//        }
//    }
//    cout << endl;

    //cout << result << endl;

}


void printString() {
    for (auto it = myString->begin(); it != myString->end(); ++it) {
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


bool dfa(int state, vector<char>::iterator it) {

    if (it == myString->end()) {
        return state == 4;
    }

    switch (state) {
        case 0:
            if (regex_match(string(1,*it), oneToNine))
                return dfa(2, ++it);
            else if (*it == '0')
                return dfa(1, ++it);
            else
                return false;
        case 1:
            if (*it == '.')
                return dfa(3, ++it);
            else
                return false;
        case 2:
            if (regex_match(string(1,*it), zeroToNine))
                return dfa(2, ++it);
            else if (*it == '.')
                return dfa(3, ++it);
            else
                return false;
        case 3:
            if (regex_match(string(1,*it), zeroToNine))
                return dfa(4, ++it);
            else
                return false;
        case 4:
            if (regex_match(string(1,*it), zeroToNine))
                return dfa(4, ++it);
            else
                return false;
        default:
            return false;
    }

}


vector<iterator_tuple> divide(int threads) {
    vector<iterator_tuple> v = vector<iterator_tuple>();
    v.resize(threads);
    int size = myString->size() / threads;
    for (int i = 0; i < threads; i++) {
        iterator_tuple t = tuple<char_iterator, char_iterator>(myString->begin()+size*i, myString->begin()+size*i+size);
        v.push_back(t);
    }
    iterator_tuple lastTuple = tuple<char_iterator, char_iterator>(myString->begin()+size*threads, myString->end());
    v.push_back(lastTuple);
    return v;
}

void process() {

}