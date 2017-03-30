//
// Created by eschaal on 3/24/17.
// Note : Not working, spent hours trying to debug without success... I think there is a problem with the way i am using iterators.
//

#include <iostream>
#include <regex>
#include <omp.h>
#include <assert.h>
#include <sys/resource.h>


#define GEN_SIZE 12


using namespace std;
typedef vector<char>::iterator char_iterator;
typedef tuple<char_iterator, char_iterator> iterator_tuple;

class Result;
class Compound;
void generateString(int length);
void printString(vector<char>* string);
void process(vector<iterator_tuple> tasks);
void printTasks(vector<iterator_tuple> tasks);
vector<iterator_tuple> divide(int threads);
bool dfa_verifier(int state, char_iterator start, char_iterator end);
Result dfa(int state, char_iterator start, char_iterator end);
void dfa_internal(int state, char_iterator start, char_iterator end, Result* r);

char generator[] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.', 'x'};

vector<char> *myString;

regex oneToNine = regex("[1-9]");
regex zeroToNine = regex("[0-9]");

class Result {

public:
    int endState;
    vector<vector<char>> final;
    vector<char> current;

};

class Compound{

public:
    Result results[5];

    void addResult(int index, Result r) {
        results[index] = r;
        return;
    }


};


bool dfa_verifier(int state, char_iterator start, char_iterator end) {
    if (start == end) {
        return state == 4;
    }


    switch (state) {

        case 0:
            if (regex_match(string(1, *start), oneToNine)) {
                return dfa_verifier(2,start+1, end);
            }
            else if (*start == '0') {
                return dfa_verifier(1, start+1, end);
            }
            else {
                return false;
            }
        case 1:
            if (*start == '.') {
                return dfa_verifier(3, start+1, end);
            } else {
                return false;
            }
        case 2:
            if (regex_match(string(1,*start), zeroToNine)) {
                return dfa_verifier(2, start+1, end);
            } else if (*start == '.') {
                return dfa_verifier(3, start+1, end);
            } else {
                return false;
            }
        case 3:
            if (regex_match(string(1, *start), zeroToNine)) {
                return dfa_verifier(4, start+1, end);
            }
            else {
                return false;
            }
        case 4:
            if (regex_match(string(1, *start), zeroToNine)) {
                return dfa_verifier(4, start+1, end);
            }
            else {
                return false;
            }
        default:
            return false;

    }
}

void dfa_internal(int state, char_iterator start, char_iterator end, Result& r) {


    if (start == end) {
        r.endState = state;
        r.final.push_back(r.current);
        return;
    }



    switch (state) {

        case 0:
            if (regex_match(string(1, *start), oneToNine)) {
                r.current.push_back(*start);
                return dfa_internal(2, start+1, end, r);
            }
            else if (*start == '0') {
                r.current.push_back(*start);
                return dfa_internal(1, start+1, end, r);
            }
            else if (*start == 'x') {
                r.current.clear();
                return dfa_internal(0, start + 1, end, r);
            }
            else {
                r.current.clear();
                return dfa_internal(0, start+1, end, r);
            }
        case 1:
            if (*start == '.') {
                r.current.push_back(*start);
                return dfa_internal(3, start+1, end, r);
            }
            else if (*start == 'x') {
                r.current.clear();
                return dfa_internal(0, start + 1, end, r);
            }
            else {
                r.current.clear();
                return dfa_internal(0, start, end, r);
            }
        case 2:
            if (regex_match(string(1,*start), zeroToNine)) {
                r.current.push_back(*start);
                return dfa_internal(2, start+1, end, r);
            } else if (*start == '.') {
                r.current.push_back(*start);
                return dfa_internal(3, start+1, end, r);
            } else if (*start == 'x') {
                r.current.clear();
                return dfa_internal(0, start + 1, end, r);
            } else {
                r.current.clear();
                return dfa_internal(0, start, end, r);
            }
        case 3:
            if (regex_match(string(1, *start), zeroToNine)) {
                r.current.push_back(*start);
                return dfa_internal(4, start+1, end, r);
            } else if (*start == 'x') {
                r.current.clear();
                return dfa_internal(0, start + 1, end, r);
            }
            else {
                r.current.clear();
                return dfa_internal(0, start, end, r);
            }
        case 4:
            if (regex_match(string(1, *start), zeroToNine)) {
                r.current.push_back(*start);
                return dfa_internal(4, start+1, end, r);
            } else if (*start == 'x') {
                r.final.push_back(r.current);
                r.current.clear();
                return dfa_internal(0, start + 1 , end, r);
            }
            else {
                r.final.push_back(r.current);
                r.current.clear();
                return dfa_internal(0, start, end, r);
            }
        default:
            return;

    }

}

Result dfa(int state, char_iterator start, char_iterator end) {

    Result r;
    dfa_internal(state, start, end, r);



    return r;


}

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
    //Result r = dfa(0, myString->begin(), myString->end());
    process(tasks);
    double stop = omp_get_wtime();


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

void printTasks(vector<iterator_tuple> v) {

    for (vector<iterator_tuple>::iterator i = v.begin(); i != v.end(); ++i) {
        for (auto j = get<0>(*i); j != get<1>(*i); ++j) {
            cout << *j << " ";
        }
    }
    cout << endl;
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
    iterator_tuple lastTuple = tuple<char_iterator, char_iterator>(myString->begin()+size*(threads-1), myString->end()+1);
    v.push_back(lastTuple);
    return v;
}


void process(vector<iterator_tuple> tasks) {
    Result result;
    Compound compounds[tasks.size()-1];
    #pragma omp parallel
    {

        #pragma omp master
        {

            result = dfa(0, get<0>(*tasks.begin()), get<1>(*tasks.begin())); // start first thread

        }
        // run external loop in parallel, 1 chunk per thread
        #pragma omp parallel for schedule(static, 1) collapse(1)
        for (uint i = 1; i < tasks.size(); i++) { // start all other threads, they match on their task starting from all possible dfa state
            {
                for (int j = 0; j < 5; j++) {
                    auto startIterator = get<0>(tasks[i]);
                    auto endIterator = get<1>(tasks[i]);
                    Result r = dfa(j, startIterator, endIterator);
                    compounds[i - 1].addResult(j, r);
                }
            }
        }

    }
    // not necessary
#pragma omp barrier

    // merge results

    int lastState = result.endState;
    vector<vector<char>> firstFloats = result.final;
    vector<vector<char>> finalResult;

    for (auto it = firstFloats.begin(); it != (firstFloats.end() - 1); it++) {
        finalResult.push_back(*it);
    }

    /**
     * try to merge results master thread and first thread
     * If not possible check if they are floats, if not, don't print
     */
    auto last = *(firstFloats.end()-1);
    auto first = *(compounds[0].results[lastState].final.begin());
    vector<char> concat = vector<char>(last.begin(), last.end());
    concat.insert(concat.end(), first.begin(), first.end());

    if (dfa_verifier(0, concat.begin(), concat.end())) {
        finalResult.push_back(concat);
        }
    else if (dfa_verifier(0, first.begin(), first.end())) {
        finalResult.push_back(first);
    }

//    if (compounds[0].results[lastState].final.size() > 1) {
//        for (auto it = (compounds[0].results[lastState].final.begin() + 1);
//             it != (compounds[0].results[lastState].final.end() - 1); it++) {
//            finalResult.push_back(*it);
//        }
//    }

    firstFloats = compounds[0].results[lastState].final;
    lastState = compounds[0].results[lastState].endState;

    /**
     * Merge other results
     */
    for (uint i = 1; i < tasks.size()-1; ++i) {

        auto last = *(firstFloats.end()-1);
        auto first = *(compounds[i].results[lastState].final.begin());
        vector<char> concat = vector<char>(last.begin(), last.end());
        concat.insert(concat.end(), first.begin(), first.end());

        if (dfa_verifier(0, concat.begin(), concat.end())) {
            finalResult.push_back(concat);
        }
        else if (dfa_verifier(0, first.begin(), first.end())) {
            finalResult.push_back(first);
        }



        if (compounds[i].results[lastState].final.size() > 1) {
            for (auto it = (compounds[i].results[lastState].final.begin() + 1);
                 it != (compounds[i].results[lastState].final.end() - 1); ++it) {
                finalResult.push_back(*it);
            }
        }

        firstFloats = compounds[i].results[lastState].final;
        lastState = compounds[i].results[lastState].endState;

    }


    // print final result

//    for (auto i = finalResult.begin(); i != finalResult.end(); ++i) {
//        for (auto it = (*i).begin(); it != (*i).end(); ++it) {
//            cout << *it << " ";
//        }
//        cout << "\t";
//    }
//
//    cout << endl;





}

