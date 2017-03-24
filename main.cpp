#include <iostream>
#include <vector>
#include <list>
#include <omp.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <time.h>


using namespace std;

struct customTuple;
void printList(std::list<char>* l);
void gen(list<char> *l, function<void(vector<customTuple>*,int, char)> rule);
function<void(vector<customTuple>*,int, char)> getRule(int index);


double start, finish;
double elapsed;

struct customTuple {
    int index;
    list<char> data;

    customTuple(int index, list<char> data) {
        this->data = data;
        this->index = index;
    }
};


int main() {

    int n = 8;

    omp_set_num_threads(1);


    list<char> *l = new list<char>();

    l->insert(l->begin(), 'a');

    start = omp_get_wtime();

    for (int i = 0; i < 4; i++) {
        gen(l, getRule(2));
        printList(l);
    }

    finish = omp_get_wtime();
    elapsed = (finish - start);

    printf("Time taken: %.2fs\n", elapsed);

    delete(l);

}

char getStartString(int i) {
    switch (i) {
        case 1:
            return 'a';
        case 2:
            return 'a';
        case 3:
            return 'a';
        case 4:
            return 'b';
    }
}

function<void(vector<customTuple>*,int, char)> getRule(int index) {
    switch (index) {
        case 1:
            return [](vector<customTuple>* l,int index, char c) {
                if (c == 'a') {
                    list<char> k;
                    k.push_back('a');
                    k.push_back('b');
                    l->push_back(customTuple(index, k));
                }
                else if (c == 'b') {
                    list<char> k;
                    k.push_back('b');
                    k.push_back('a');
                    l->push_back(customTuple(index, k));
                }
            };
        case 2:
            return [](vector<customTuple>* l,int index, char c) {
                if (c == 'a') {
                    list<char> k;
                    k.push_back('b');
                    l->push_back(customTuple(index, k));
                }
                else if (c == 'b') {
                    list<char> k;
                    k.push_back('b');
                    k.push_back('a');
                    l->push_back(customTuple(index, k));
                }
            };
        case 3:
            return [](vector<customTuple>* l,int index, char c) {
                if (c == 'a') {
                    list<char> k;
                    k.push_back('a');
                    k.push_back('b');
                    k.push_back('a');
                    l->push_back(customTuple(index, k));
                }
                else if (c == 'b') {
                    list<char> k;
                    k.push_back('b');
                    k.push_back('b');
                    k.push_back('b');
                    l->push_back(customTuple(index, k));
                }
            };
        case 4:
            return [](vector<customTuple>* l,int index, char c) {
                if (c == 'a') {
                    list<char> k;
                    k.push_back('a');
                    k.push_back('c');
                    l->push_back(customTuple(index, k));
                }
                else if (c == 'b') {
                    list<char> k;
                    k.push_back('a');
                    k.push_back('b');
                    k.push_back('c');
                    l->push_back(customTuple(index, k));
                }
                else if (c == 'c') {
                    list<char> k;
                    k.push_back('c');
                    k.push_back('a');
                    l->push_back(customTuple(index, k));
                }
            };
        default:
            return nullptr;
    }
}

void rule(vector<customTuple>* l,int index, char c) {
    if (c == 'a') {
        list<char> k;
        k.push_back('a');
        k.push_back('b');
        l->push_back(customTuple(index, k));
    }
    else if (c == 'b') {
        list<char> k;
        k.push_back('b');
        k.push_back('a');
        l->push_back(customTuple(index, k));
    }
}


void gen(list<char> *l, function<void(vector<customTuple>*,int, char)> rule) {
    vector<customTuple> priv = vector<customTuple>();

    #pragma omp declare reduction (merge: vector<customTuple> : omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()))

    #pragma omp parallel for reduction(merge: priv)
    for (int i = 0; i < l->size(); i++) {
        auto it = l->begin();

        for (int j = 1; j <= i; j++)
            it++;
        rule(&priv, i, *it);
    }

    sort(priv.begin(), priv.end(),[](customTuple a, customTuple b) { return a.index < b.index;});
    l->clear();
    for (auto i = priv.begin(); i != priv.end(); ++i) {
        for (auto it = (*i).data.begin(); it != (*i).data.end(); ++it)
            l->push_back(*it);
    }

}

void printList(list<char>* l) {
    for (auto it = l->begin(); it != l->end(); ++it) {
        cout << *it << " ";
    }
    cout << endl;
}


