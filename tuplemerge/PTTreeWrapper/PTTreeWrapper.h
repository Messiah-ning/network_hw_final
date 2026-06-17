#pragma once
#include "../Simulation.h"

class PTTreeWrapper : public PacketClassifier {
    struct Impl;
    Impl* impl;
public:
    PTTreeWrapper();
    ~PTTreeWrapper();

    void ConstructClassifier(const std::vector<Rule>& rules) override;
    int  ClassifyAPacket(const Packet& p) override;
    void DeleteRule(size_t index) override {}
    void InsertRule(const Rule& rule) override {}
    Memory MemSizeBytes() const override;
    int  MemoryAccess() const override { return 0; }
    size_t NumTables() const override { return 1; }
    size_t RulesInTable(size_t i) const override { return 0; }
    size_t PriorityOfTable(size_t i) const override { return 0; }
};
