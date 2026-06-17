// This file includes ONLY MultilayerTuple headers — never tuplemerge headers.
// Isolated to avoid Rule/Trace naming conflicts between the two frameworks.
#include "MLTBridge.h"
#include <vector>

// MLT global (defined in multilayertuple.cpp, must be set before Init)
extern int prefix_dims_num;

#include "../../MultilayerTuple/elementary.h"
#include "../../MultilayerTuple/methods/multilayertuple/multilayertuple.h"

using namespace std;

struct MLTHandle {
    MultilayerTuple*  classifier;
    vector<Rule*>     rules;
};

void* mlt_create(const MLTRuleData* data, int n_rules) {
    prefix_dims_num = 5;
    MLTHandle* h = new MLTHandle();
    h->rules.reserve(n_rules);
    for (int i = 0; i < n_rules; i++) {
        Rule* r = new Rule();
        for (int d = 0; d < 5; d++) {
            r->range[d][0]  = data[i].range[d][0];
            r->range[d][1]  = data[i].range[d][1];
            r->prefix_len[d] = data[i].prefix_len[d];
        }
        r->priority = data[i].priority;
        h->rules.push_back(r);
    }
    h->classifier = new MultilayerTuple();
    h->classifier->Init(5, true);
    h->classifier->Create(h->rules, true);
    return h;
}

int mlt_lookup(void* handle, const uint32_t key[5]) {
    MLTHandle* h = static_cast<MLTHandle*>(handle);
    Trace t;
    for (int i = 0; i < 5; i++) t.key[i] = key[i];
    return h->classifier->Lookup(&t, -1);
}

uint64_t mlt_memsize(void* handle) {
    return static_cast<MLTHandle*>(handle)->classifier->MemorySize();
}

int mlt_num_tuples(void* handle) {
    MLTHandle* h = static_cast<MLTHandle*>(handle);
    return h->classifier->tuples_num;
}

void mlt_free(void* handle) {
    MLTHandle* h = static_cast<MLTHandle*>(handle);
    h->classifier->Free(true);
    for (auto r : h->rules) delete r;
    delete h;
}
