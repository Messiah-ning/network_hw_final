// This file includes ONLY PT-Tree headers — never tuplemerge headers.
// Isolated to avoid Rule/Packet naming conflicts between the two frameworks.
#include "PTBridge.h"
#include <vector>
#include <cstdio>
#include <cstring>

#include "../../PT-Tree/src/data_structure.h"
#include "../../PT-Tree/src/pt_tree.h"

using namespace std;

struct PTHandle {
    PTtree* tree;
};

// Use a fixed config: [sip_byte0, dip_byte0] + dst_port.
// Skips config search to avoid rebuilding 100K-rule trees 372+ times.
// Fields 0-3 = source_ip bytes, 4-7 = destination_ip bytes (PT-Tree convention).
static vector<uint8_t> default_fields() {
    return {0, 4};  // sip[0], dip[0]
}
static const int DEFAULT_PORT_FIELD = 1; // destination port

// PT-Tree stores IPs in little-endian byte order.
// On x86 (little-endian), memcpy from uint32 naturally produces little-endian bytes.
static inline void store_ip_le(unsigned char dst[4], uint32_t ip) {
    memcpy(dst, &ip, 4);
}

void* pt_create(const PTRuleData* data, int n_rules) {
    PTHandle* h = new PTHandle();

    vector<Rule> rules(n_rules);
    for (int i = 0; i < n_rules; i++) {
        Rule& r = rules[i];
        memset(&r, 0, sizeof(Rule));
        r.pri = data[i].priority;

        store_ip_le(r.source_ip, data[i].src_ip);
        r.source_mask = data[i].src_prefix;

        store_ip_le(r.destination_ip, data[i].dst_ip);
        r.destination_mask = data[i].dst_prefix;

        r.source_port[0] = data[i].src_port[0];
        r.source_port[1] = data[i].src_port[1];
        r.destination_port[0] = data[i].dst_port[0];
        r.destination_port[1] = data[i].dst_port[1];

        // PT-Tree: protocol[0]=mask, protocol[1]=value
        r.protocol[0] = data[i].proto_mask;
        r.protocol[1] = data[i].protocol;
    }

    vector<uint8_t> fields = default_fields();
    h->tree = new PTtree(fields, DEFAULT_PORT_FIELD);
    for (auto& r : rules) h->tree->insert(r);

    return h;
}

int pt_lookup(void* handle, uint32_t src_ip, uint32_t dst_ip,
              uint16_t src_port, uint16_t dst_port, uint8_t protocol) {
    PTHandle* h = static_cast<PTHandle*>(handle);
    Packet p;
    memset(&p, 0, sizeof(p));
    p.protocol = protocol;
    // Same little-endian memcpy used by PT-Tree's read_packets
    store_ip_le(p.source_ip, src_ip);
    store_ip_le(p.destination_ip, dst_ip);
    p.source_port = src_port;
    p.destination_port = dst_port;
    return h->tree->search(p);
}

uint64_t pt_memsize(void* handle) {
    return (uint64_t)static_cast<PTHandle*>(handle)->tree->mem();
}

void pt_free(void* handle) {
    PTHandle* h = static_cast<PTHandle*>(handle);
    delete h->tree;
    delete h;
}
