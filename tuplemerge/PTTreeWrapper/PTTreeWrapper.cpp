#include "PTTreeWrapper.h"
#include "PTBridge.h"

struct PTTreeWrapper::Impl {
    void* handle = nullptr;
};

PTTreeWrapper::PTTreeWrapper() : impl(new Impl()) {}

PTTreeWrapper::~PTTreeWrapper() {
    if (impl->handle) pt_free(impl->handle);
    delete impl;
}

void PTTreeWrapper::ConstructClassifier(const std::vector<Rule>& rules) {
    int n = (int)rules.size();
    std::vector<PTRuleData> data(n);

    for (int i = 0; i < n; i++) {
        const Rule& r = rules[i];
        PTRuleData& d = data[i];

        d.priority  = r.priority;
        d.src_ip     = r.range[FieldSA][LowDim];
        d.src_prefix = (uint8_t)r.prefix_length[FieldSA];
        d.dst_ip     = r.range[FieldDA][LowDim];
        d.dst_prefix = (uint8_t)r.prefix_length[FieldDA];

        d.src_port[0] = (uint16_t)r.range[FieldSP][LowDim];
        d.src_port[1] = (uint16_t)r.range[FieldSP][HighDim];
        d.dst_port[0] = (uint16_t)r.range[FieldDP][LowDim];
        d.dst_port[1] = (uint16_t)r.range[FieldDP][HighDim];

        // Convert protocol range [lo, hi] → (value, mask).
        // ClassBench rules are typically exact match (lo==hi) or wildcard (0..255).
        uint32_t plo = r.range[FieldProto][LowDim];
        uint32_t phi = r.range[FieldProto][HighDim];
        uint32_t span = phi - plo;
        d.protocol   = (uint8_t)plo;
        d.proto_mask = (uint8_t)(~span & 0xFF);
    }

    impl->handle = pt_create(data.data(), n);
}

int PTTreeWrapper::ClassifyAPacket(const Packet& p) {
    QueryUpdate(1);
    return pt_lookup(impl->handle,
                     p[FieldSA],
                     p[FieldDA],
                     (uint16_t)p[FieldSP],
                     (uint16_t)p[FieldDP],
                     (uint8_t) p[FieldProto]);
}

Memory PTTreeWrapper::MemSizeBytes() const {
    return (Memory)pt_memsize(impl->handle);
}
