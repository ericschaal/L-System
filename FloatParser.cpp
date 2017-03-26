//
// Created by eschaal on 3/24/17.
// Note : Not working, spent hours trying to debug without success... I think there is a problem with the way i am using iterators.
//

#include <iostream>
#include <regex>
#include <omp.h>
#include <assert.h>


#define GEN_SIZE 12


using namespace std;
typedef vector<char>::iterator char_iterator;
typedef tuple<char_iterator, char_iterator> iterator_tuple;


class DfaResult;
class Compound;

void dfa(int state, vector<char>::iterator begin, vector<char>::iterator end, DfaResult* r, char_iterator lastFloat);
void generateString(int length);
void printString(vector<char>* string);
void process(vector<iterator_tuple> tasks);
vector<iterator_tuple> divide(int threads);
void deleteAll(char_iterator from, char_iterator to);

char generator[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'x'};

vector<char> *myString;

regex oneToNine = regex("[1-9]");
regex zeroToNine = regex("[0-9]");


class DfaResult {

public:
    int state;
    vector<function<void()>> functions;
    char_iterator lastFloat;

    void addDelete(char_iterator from, char_iterator to) {
        function<void()> func = [=]() -> void {
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


int main(int argc, char *argv[]) {

    if (argc != 3) {
        cout << "Usage : FloatParser n l" << endl;;
        cout << endl;
        cout << "n: number of optimistic threads" << endl;
        cout << "l: length of the random string" << endl;
        cout << endl;
        return EXIT_FAILURE;
    }

    srand((unsigned)time(0)); // seed

    int threads = atoi(argv[1]);
    int stringLength = atoi(argv[2]);

    assert(stringLength >= (threads+1));

    omp_set_num_threads(threads); // setting max number of threads
    myString = new vector<char>(); // string holder

    generateString(stringLength); // generate random string
    //printString(myString); // print original string
    vector<iterator_tuple> tasks = divide(threads+1); // generate tasks
    double start = omp_get_wtime();
    process(tasks);
    double stop = omp_get_wtime();
    //printString(myString);
    printf("Time taken: %.2fs\n", stop-start);
    return EXIT_SUCCESS;
}


void printString(vector<char>* string) {
    for (auto it = string->begin(); it != string->end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}

/**
 * Generate a random string
 * @param length length of the string
 */
void generateString(int length) {
    for (int i = 0; i <length; i++) {
        char c = generator[rand() % GEN_SIZE];
        myString->push_back(c);
    }
}


/**
 * Dfa to check reg exp.
 * @param state start state
 * @param begin start position
 * @param end  end position
 * @param r result pointer
 * @param lastFloat position of last known float
 */
void dfa(int state, vector<char>::iterator begin, vector<char>::iterator end, DfaResult* r, char_iterator lastFloat) {

    r->state = state;
    r->lastFloat = lastFloat;

    if (begin > end) {
        return;
    }

    switch (state) {
        case 0:
            if (regex_match(string(1,*begin), oneToNine))
                return dfa(2, begin+1, end, r, lastFloat);
            else if (*begin == '0')
                return dfa(1, begin+1, end, r, lastFloat);
            else {
                r->addDelete(lastFloat+1, begin);
                return dfa(0, begin+1, end, r, lastFloat);
            }
        case 1:
            if (*begin == '.')
                return dfa(3, begin+1, end, r, lastFloat);
            else {
                r->addDelete(lastFloat+1, begin);
                return dfa(0, begin+1, end, r, lastFloat);
            }
        case 2:
            if (regex_match(string(1,*begin), zeroToNine))
                return dfa(2, begin+1, end, r, lastFloat);
            else if (*begin == '.')
                return dfa(3, begin+1, end, r, lastFloat);
            else {
                r->addDelete(lastFloat+1, begin);
                return dfa(0, begin+1, end, r, lastFloat);
            }
        case 3:
            if (regex_match(string(1,*begin), zeroToNine)) {
                return dfa(4, begin + 1, end, r, lastFloat);
            }
            else {
                r->addDelete(lastFloat+1, begin);
                return dfa(0, begin+1, end, r, lastFloat);
            }
        case 4:
            if (regex_match(string(1,*begin), zeroToNine)) {
                lastFloat = begin;
                return dfa(4, begin + 1, end, r, lastFloat);
            }
            else {
                r->addDelete(lastFloat+1, begin);
                return dfa(0, begin+1, end, r, lastFloat);
            }


    }
}

/**
 * Replace all characters in range
 * @param from inclusive start iterator
 * @param to inclusive end iterator
 */
void deleteAll(char_iterator from, char_iterator to) {
    for (auto it = from; it <= to; it++) {
        *it = ' ';
    }
}

/**
 * Divide string into multiple tasks
 * @param threads
 * @return tuple of a start iterator and end iterator
 */
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

        #pragma omp single
        {

            dfa(0, get<0>(*tasks.begin()), get<1>(*tasks.begin()), &result, myString->begin()-1); // start first thread

        }
        #pragma omp parallel for schedule(static, 1) collapse(1)
        for (uint i = 1; i < tasks.size(); i++) { // start all other threads, they match on their task starting from all possible dfa state
            {
                for (int j = 0; j < 5; j++) {
                    auto startIterator = get<0>(tasks[i]);
                    auto endIterator = get<1>(tasks[i]);
                    DfaResult r = DfaResult();
                    dfa(j, startIterator, endIterator, &r, myString->end());
                    compounds[i - 1].addResult(j, r);
                }
            }
        }

    }
    #pragma omp barrier

    result.exec(); // remove non float from first task.
    int lastState = result.state;
    for (auto compound: compounds) {
        auto resultFromState = compound.results[lastState]; // select the right starting state
        resultFromState.exec(); // remove non float in string
        lastState = resultFromState.state; // update state for next task
    }


}