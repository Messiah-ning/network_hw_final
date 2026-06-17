#pragma once
#include <stdint.h>

struct MLTRuleData {
    uint32_t range[5][2];
    char     prefix_len[5];
    int      priority;
};

void*    mlt_create(const MLTRuleData* rules, int n_rules);
int      mlt_lookup(void* handle, const uint32_t key[5]);
uint64_t mlt_memsize(void* handle);
int      mlt_num_tuples(void* handle);
void     mlt_free(void* handle);
