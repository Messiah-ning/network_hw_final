#include "MultilayerTupleWrapper.h"
#include "MLTBridge.h"

struct MultilayerTupleWrapper::Impl {
    void*  handle = nullptr;
    int    num_tuples = 0;
};

MultilayerTupleWrapper::MultilayerTupleWrapper() : impl(new Impl()) {}

MultilayerTupleWrapper::~MultilayerTupleWrapper() {
    if (impl->handle) mlt_free(impl->handle);
    delete impl;
}

void MultilayerTupleWrapper::ConstructClassifier(const std::vector<Rule>& rules) {
    int n = (int)rules.size();
    std::vector<MLTRuleData> data(n);
    for (int i = 0; i < n; i++) {
        for (int d = 0; d < 5; d++) {
            data[i].range[d][0] = rules[i].range[d][LowDim];
            data[i].range[d][1] = rules[i].range[d][HighDim];
            data[i].prefix_len[d] = (char)rules[i].prefix_length[d];
        }
        data[i].priority = rules[i].priority;
    }
    impl->handle = mlt_create(data.data(), n);
    impl->num_tuples = mlt_num_tuples(impl->handle);
}

int MultilayerTupleWrapper::ClassifyAPacket(const Packet& p) {
    uint32_t key[5];
    for (int i = 0; i < 5; i++) key[i] = p[i];
    QueryUpdate(1);
    return mlt_lookup(impl->handle, key);
}

Memory MultilayerTupleWrapper::MemSizeBytes() const {
    return (Memory)mlt_memsize(impl->handle);
}

size_t MultilayerTupleWrapper::NumTables() const {
    return (size_t)impl->num_tuples;
}
