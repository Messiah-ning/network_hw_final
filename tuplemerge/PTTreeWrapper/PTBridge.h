#pragma once
#include <stdint.h>

struct PTRuleData {
    uint32_t src_ip;      // network address as uint32 (host byte order, MSB = octet1)
    uint8_t  src_prefix;
    uint32_t dst_ip;
    uint8_t  dst_prefix;
    uint16_t src_port[2]; // [lo, hi]
    uint16_t dst_port[2]; // [lo, hi]
    uint8_t  protocol;    // protocol value
    uint8_t  proto_mask;  // protocol mask (0xFF = exact, 0x00 = wildcard)
    int      priority;
};

void*    pt_create(const PTRuleData* rules, int n_rules);
int      pt_lookup(void* handle, uint32_t src_ip, uint32_t dst_ip,
                   uint16_t src_port, uint16_t dst_port, uint8_t protocol);
uint64_t pt_memsize(void* handle);
void     pt_free(void* handle);
