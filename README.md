# Packet Classification Algorithm Benchmark

以 100K ClassBench ACL1 規則集為測試資料，比較四種封包分類演算法的建表時間、查找時間與記憶體用量。

---

## 演算法

| 演算法 | 說明 |
|---|---|
| **TupleMerge-Online** | Tuple Space Search 的改良版，online 建表 |
| **TupleMerge-Offline** | TupleMerge offline 版本，事先優化 table 分配 |
| **MultilayerTuple** | 多層 Tuple，將規則分散至多個 hash table |
| **PT-Tree** | Prefix Tree，以 IP prefix byte 為節點的樹狀結構 |

原始碼來源：
- `tuplemerge/`：[TupleMerge](https://github.com/drjeong/tuplemerge)（含本專案新增的 Wrapper）
- `MultilayerTuple/`：MultilayerTuple 演算法原始碼
- `PT-Tree/`：[PT-Tree](https://github.com/SJTU-IPADS/PT-Tree) 原始碼

---

## Benchmark 結果（100K ACL1 rules，1M packets）

| 演算法 | Build Time | Lookup Time | Memory |
|---|---|---|---|
| TupleMerge-Online | 230 ms | 0.48 s | 2.35 MB |
| TupleMerge-Offline | 710 ms | 2.57 s | 2.30 MB |
| MultilayerTuple | 30 ms | 9.88 s | 12.17 MB |
| PT-Tree | 5 ms | 0.02 s | 2.35 MB |

完整輸出：[Output_TupleMerge/100K_acl1.rules.csv](Output_TupleMerge/100K_acl1.rules.csv)

---

## 環境需求

- g++ (支援 C++14)
- OpenMP（`-fopenmp`）
- Python 3（生成測資用）

---

## 使用方式

### 1. 生成測資

100K 規則與封包不在 git 中，需先用腳本生成：

```bash
# 生成 100K 規則（從 MultilayerTuple 內建的 10K 規則取樣）
python3 gen_rules.py MultilayerTuple/data/10K_acl1_rules \
        tuplemerge/data/100K_acl1.rules --count 100000

# 生成 1M 封包 trace
python3 gen_traces.py tuplemerge/data/100K_acl1.rules \
        tuplemerge/data/100K_acl1.rules.trace
```

### 2. 編譯

```bash
cd tuplemerge
make
```

### 3. 執行 Benchmark

```bash
cd tuplemerge
bash test.sh
```

結果輸出至終端機，並寫入 `../Output_TupleMerge/100K_acl1.rules.csv`。

---

## 專案結構

```
.
├── tuplemerge/                  # 整合框架（主要入口）
│   ├── MultilayerTupleWrapper/  # MultilayerTuple 整合 wrapper
│   ├── PTTreeWrapper/           # PT-Tree 整合 wrapper
│   ├── makefile
│   └── test.sh                  # 執行所有演算法的統一入口
├── MultilayerTuple/             # MultilayerTuple 原始碼
├── PT-Tree/                     # PT-Tree 原始碼
├── Output_TupleMerge/           # Benchmark 輸出結果
├── gen_rules.py                 # 測資生成：規則
└── gen_traces.py                # 測資生成：封包 trace
```

---

## 自訂演算法組合

編輯 `tuplemerge/test.sh` 第 4 行：

```sh
# 可用名稱：PartitionSort, PriorityTuple, TMOffline, TMOnline,
#           PartitionSortOffline, MultilayerTuple, PTTree
Classifiers="TMOffline,TMOnline,MultilayerTuple,PTTree"
```
