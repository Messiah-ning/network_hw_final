#!/bin/sh
# Classifiers="PartitionSort,PriorityTuple,TMOffline,TMOnline,PartitionSortOffline,MultilayerTuple,PTTree"

Classifiers="TMOffline,TMOnline,MultilayerTuple,PTTree"

RuleList="100K_acl1"
SourceDir=./data
PacketDir=./data
OutputDir=../Output_TupleMerge/

Limit=10

echo $f #
if [ ! -d ${OutputDir} ]; then
	mkdir -p ${OutputDir}/
fi

File=${SourceDir}/${RuleList}.rules
Output=${OutputDir}/${RuleList}.rules.csv
Packets=${PacketDir}/${RuleList}.rules.trace
./main f=${File} c=${Classifiers} o=${Output} TM.Limit.Collide=${Limit} p=${Packets}

