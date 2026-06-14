/*
对于大模型训练/推理的过程，对于全局内存访问的抽象可以用以下四元组的序列来表达： [addr0
, size0
, start0
, time0
], [addr1
, size1
, start1
, time1
], … [addrn−1
, sizen−1
, startn−1
, timen−1
] addr和size定义了一段内存，表示这段内存的起始地址和大小。 start和time定义了这段内存被访问的时间段，表示开始访问的起始时间和持续时间。 有以下几点需要注意：

同一段内存（的子集）可能被反复访问，并且不同的内存段之间可能存在交集。
内存访问时间满足start0≤start1≤...≤startn−1
，这也意味着不同段内存的访问可能是同时发生的。
内存访问序列的每个四元组都被认为是一个计算过程，这个过程可以和内存卸载/加载过程并行，并且不影响彼此的执行时间。
内存卸载和加载过程，不能并行。
在初始状态时（第一条内存序列执行前），内存都是空闲的，所有的数据都未加载到内存。
访问序列中，starti
相等的条目应视为同一计算任务发出的请求。同一计算任务的开始时间和持续时间均相同，且同一计算任务的内存访问可以并行完成，而不同计算任务之间只能串行完成。同一计算任务的内存访问操作必须同时开始，同时结束。详见输入输出示例三。
三、期望达到的目标和效果
对于给定的内存访问序列，需要在序列中插入适当的内存卸载/加载操作，以满足以下目标：

目标一：瞬时的最大内存占用，不能超过内存的最大容量。
目标二：对于任意一段内存，在被访问的时间段内，必须固定在内存中（即这段内存不能处于被卸载的状态）。
四、用例描述
输入格式：

第一行输入三个整数L
，M
，N
，其中L
表示进程总虚拟地址大小，M
表示HBM总容量，N
表示内存访问序列长度（1≤M≤L≤100000
; 1≤N≤10000
）。接下来N
行，每行4个整数addri
, sizei
, starti
, timei
 （0≤addri<L,1≤sizei≤L
; addri+sizei≤L
; 0≤starti,timei≤109
）描述一次内存访问。其中addri
表示该段内存的起始虚拟地址，sizei
表示该段虚拟地址大小，starti
表示该段内存的最早可访问时间，timei
表示该段内存被访问的持续时间。输入保证若存在i,j
满足starti=startj
，则timei=timej
。输入保证starti
是单调不减的。

输出格式：

输出最多10×N
行，每一行表示一个执行操作，分为四种类型：

Reload Ti
 Ai
 Si
，表示预取操作，意为在Ti
时刻把起始虚拟地址为Ai
，大小为Si
的内存载入HBM，总共花费时间40×Si
（其中已存在HBM的部分不会重复载入，不计时间开销）。需要保证0≤Ai<L,1≤Si≤L
，且Ai+Si≤L
。
Visit Ti
 Ai
，表示访存操作，意为在Ti
时刻开始进行第Ai
个内存访问诉求，总共花费时间为timeAi
。需要保证0≤Ai<N
，且访存操作对应的计算任务完成顺序应与输入顺序相同。
Offload Ti
 Ai
 Si
，表示卸载操作，意为在Ti
时刻把起始虚拟地址为Ai
，大小为Si
的内存从HBM中释放，总共花费时间40×Si
（其中已释放的部分不会重复释放，不计时间开销）。需要保证0≤Ai<L,1≤Si≤L
，且Ai+Si≤L
。
Fin T
，表示任务结束，意为所有计算任务全部完成的时间为T
。需要保证该操作只在最后一行输出一次。
其中操作1和操作3为读写类型操作，操作2为访存类型操作。读写类型操作和访存类型操作可以并行完成，同一计算任务的访存操作也可以并行完成。但是读写类型的操作只能串行完成，非同一计算任务的访存类型操作也只能串行完成。

输入示例一：
200 100 2
0 100 0 30
100 100 50 10

输出示例一：
Reload 0 0 100
Visit 4000 0
Offload 4030 0 100
Reload 8030 100 100
Visit 12030 1
Fin 12040

示例一说明：

初始时间T=0
，所有虚拟地址空间均不在GPU/NPU内存中，内存容量为100
。
由于序号0
的内存访问需要用到[0,100)
范围的虚拟地址，所以输出Reload操作把它们载入内存。Reload操作加载100
单位内存，花费4000
单位时间，此时T=4000
，虚拟地址[0,100)
的数据已经在内存中。
序号0
的内存访问最早开始时间为0≤T
，可以直接开始序号0
的内存访问，输出Visit操作，花费30
个单位的时间，此时T=4030
。
由于此时内存已满，执行序号1的访问前需要先做offload操作。虚拟地址空间[0,100)
完成offload，此时T=8030
。
把虚拟地址[100,200)
的数据载入内存，花费4000
个单位时间，此时T=12030
。
序号1
的内存访问最早开始时间为50≤T
，可以直接开始序号1
的内存访问，输出Visit操作，花费10
个单位的时间，此时T=12040
。
所有内存访问操作都已完成，输出FIN，花费总时间T=12040
。
输入示例二：
300 200 3
0 100 0 50
100 100 4000 30
150 100 4001 20

输出示例二：
Reload 0 0 100
Visit 4000 0
Reload 4000 100 100
Visit 8000 1
Offload 8000 0 50
Reload 10000 200 50
Visit 12000 2
Fin 12020

示例二说明：
注意，读写操作和访存操作是可以并行的，因此在时刻T=4000
时，可以同时开始Visit和Reload操作。由于内存访问序号1、2的虚拟地址存在重合部分，
做offload操作时不需要把序号1所访问的内存全部offload。

输入示例三：
300 200 3
0 100 0 5000
100 100 0 5000
50 100 4001 20

输出示例三：
Reload 0 100 100
Reload 4000 0 100
Visit 8000 1
Visit 8000 0
Visit 13000 2
Fin 13020

示例三说明：
由于内存访问序号0、1的start时间是相同的，所以它们属于同一个计算任务的内存访问，
且输出顺序无严格限制，但是必须同时开始，同时完成。
内存访问序号2的start时间与0、1不同，应视为不同的计算任务发出的，因此需要等待之前的计算任务完成后才能开始。-------------
-------------
解题思路
-------------
1.先把输入分组，每组的start时间相同，表示同一个计算任务内的访问，每组的计算任务之间可以并行计算。
2.从空内存开始进行读入操作，每次读入前判断当前内存中已经加载的区间和需要读入的区间，计算出需要读入的区间；
    1.若需要读入的区间加上当前内存中已经加载的区间超过了内存限制M，则把这个任务放入等待列表里面，等到当前内存中有足够空间的时候再进行读入操作；
    2.进行读入操作，读入时间为读入区间长度的40倍，更新当前内存状态。
    3.每当一个任务的读入操作完成后，开始进行计算任务，计算任务的时长为timei，计算任务可以并行进行，因此只需要维护一个当前计算任务的结束时间即可。
3.当前一组读入操作已经做完，且内存还有剩余的情况下，提前预取下一组的读入操作。
    1.每当一个任务的计算任务完成后，先把这个任务的内存区间和下一组任务不重合的地方放入需要卸载的内存。判断下一组任务哪个任务和这个任务的内存区间重合度最高，就尝试预取这个任务的读写操作。
    （不能分段预取，内存中有足够空间）
    2.维持一个预取序列（优先队列，优先级是任务做完的时间越长越先预取）
    3.每当读写操作空闲时，先判断是否还有需要卸载的内存，若有则继续卸载；若没有，则在内存有空闲的时候检查预取序列，进行预取。
4.等当前组计算任务做完后，直接开始进行下一组的计算任务。（计算任务可以并行，因此只需要判断对应的读写任务是否做完即可）
5.当所有任务的计算任务做完（不用卸载所有内存），输出完成时间。
------------
新思路
------------
访存操作基本不会被限制，所以主要是读入操作
还是分组，读入操作的优先级是（当内存空的时候）：先加载当前组中还没加载的job；如果都加载完了优先卸载已经访存完并且
和下一组不重合的的内存；如果还有空间就按输入顺序预取下一组的job
-----------
思路整合
-----------
1.分组，按组处理
2.每组处理时，先计算当前组需要加载的区间need（保证不超过内存容量M）
3.先卸载掉所有用不到的区间，释放内存空间以加载need
4.在当前组执行时，试图预取下一组的区间（按输入顺序），以减少后续等待时间
5.执行结束后，进行下一组
*/

#include <iostream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <sstream>
#include <algorithm>
#include <vector>
#include <climits>
using namespace std;

struct Access {
    int addr; 
    int size; 
    long long start; 
    long long time; 
};

static const bool DEBUG = false;
static void printIntervals(const string &tag, const vector<pair<int,int>> &v) {
    if (!DEBUG) return;
    cerr << tag << ":";
    for (auto &p : v) cerr << " [" << p.first << "," << p.second << ")";
    cerr << " \n";
}
static void printLoaded(const vector<pair<int,int>> &loaded, long long rw_time, long long comp_time, long long cur_loaded, int M) {
    if (!DEBUG) return;
    cerr << "STATE rw_time=" << rw_time << " comp_time=" << comp_time << " cur_loaded=" << cur_loaded << " M=" << M << "\n";
    printIntervals("  loaded", loaded);
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int L, M, N;
    if (!(cin >> L >> M >> N)) return 0;
    vector<Access> a(N);
    for (int i = 0; i < N; ++i) cin >> a[i].addr >> a[i].size >> a[i].start >> a[i].time;
    
    // group by equal start (consecutive)
    vector<vector<int>> groups;
    for (int i = 0; i < N; ) {
        int j = i+1;
        while (j < N && a[j].start == a[i].start) ++j;
        groups.emplace_back();
        for (int k = i; k < j; ++k) groups.back().push_back(k);
        i = j;
    }
    if (DEBUG) {
        cerr << "Groups (" << groups.size() << "):\n";
        for (size_t gi = 0; gi < groups.size(); ++gi) {
            cerr << " group " << gi << " start=" << a[groups[gi].front()].start << " idxs:";
            for (int idx : groups[gi]) cerr << " " << idx;
            cerr << "\n";
        }
    }
    // loaded intervals as sorted non-overlapping [l,r)
    vector<pair<int,int>> loaded;
    auto merge_insert = [&](int Lx, int Rx){
        if (Lx >= Rx) return;
        vector<pair<int,int>> res;
        int l = Lx, r = Rx;
        bool placed = false;
        for (auto &it : loaded) {
            if (it.second < l) res.push_back(it);
            else if (it.first > r) {
                if (!placed) { res.push_back({l,r}); placed = true; }
                res.push_back(it);
            } else {
                l = min(l, it.first);
                r = max(r, it.second);
            }
        }
        if (!placed) res.push_back({l,r});
        loaded.swap(res);
    };
    auto remove_interval = [&](int Lx, int Rx){
        if (Lx >= Rx) return;
        vector<pair<int,int>> res;
        for (auto &it : loaded) {
            if (it.second <= Lx || it.first >= Rx) res.push_back(it);
            else {
                if (it.first < Lx) res.push_back({it.first, Lx});
                if (it.second > Rx) res.push_back({Rx, it.second});
            }
        }
        loaded.swap(res);
    };
    auto total_loaded = [&](){
        long long s = 0;
        for (auto &it : loaded) s += (it.second - it.first);
        return s;
    };
    auto merge_intervals = [&](vector<pair<int,int>> v){
        if (v.empty()) return v;
        sort(v.begin(), v.end());
        vector<pair<int,int>> res;
        res.push_back(v[0]);
        for (size_t i = 1; i < v.size(); ++i) {
            if (v[i].first < res.back().second) res.back().second = max(res.back().second, v[i].second);
            else res.push_back(v[i]);
        }
        return res;
    };
    auto build_req = [&](const vector<int>& g)->vector<pair<int,int>> {
        vector<pair<int,int>> req;
        for (int idx : g) req.emplace_back(a[idx].addr, a[idx].addr + a[idx].size);
        return merge_intervals(req);
    };
    auto needed_parts = [&](const vector<pair<int,int>>& req)->vector<pair<int,int>> {
        vector<pair<int,int>> need;
        for (auto &r : req) {
            int cur = r.first;
            for (auto &it : loaded) {
                if (it.second <= cur) continue;
                if (it.first >= r.second) break;
                if (it.first > cur) { need.emplace_back(cur, min(r.second, it.first)); cur = min(r.second, it.second); }
                else cur = max(cur, it.second);
                if (cur >= r.second) break;
            }
            if (cur < r.second) need.emplace_back(cur, r.second);
        }
        return merge_intervals(need);
    };

    // precompute group reqs
    vector<vector<pair<int,int>>> group_reqs(groups.size());
    for (size_t gi = 0; gi < groups.size(); ++gi) group_reqs[gi] = build_req(groups[gi]);
    vector<long long> group_end(groups.size(), 0); // visit end times

    long long rw_time = 0;   // serialized reload/offload timeline
    long long comp_time = 0; // serialized visits across groups
    vector<string> out;

    for (size_t gi = 0; gi < groups.size(); ++gi) {
        auto &g = groups[gi];
        auto req = group_reqs[gi];
        if (DEBUG) {
            cerr << "----------------------------------------\n";
            cerr << "Processing group " << gi << " (start=" << a[g.front()].start << ")\n";
            printLoaded(loaded, rw_time, comp_time, total_loaded(), M);
            printIntervals("  req", req);
        }
        // compute what needs to be loaded
        auto need = needed_parts(req);
        long long need_bytes = 0;
        for (auto &p : need) need_bytes += (p.second - p.first);
        if (DEBUG) {
            printIntervals("  need (missing)", need);
            cerr << "  need_bytes=" << need_bytes << "\n";
        }
        long long cur_loaded = total_loaded();

        // try to free space by offloading intervals that do NOT overlap any currently protected data
        auto make_forbidden = [&](long long time_point)->vector<pair<int,int>> {
            // protect: current req + any group whose visit end > time_point (still active at time_point)
            vector<pair<int,int>> forb = req;
            for (size_t k = 0; k < groups.size(); ++k) {
                if (k == gi) continue;
                if (group_end[k] > time_point) {
                    forb.insert(forb.end(), group_reqs[k].begin(), group_reqs[k].end());
                }
            }
            return merge_intervals(forb);
        };

        if (need_bytes + cur_loaded > M) {
            // Phase 1: offload parts not overlapping currently protected intervals at current rw_time
            auto forbidden = make_forbidden(rw_time);
            vector<pair<int,int>> freeable;
            for (auto &seg : loaded) {
                int Lx = seg.first, Rx = seg.second, cur = Lx;
                for (auto &f : forbidden) {
                    if (f.second <= cur) continue;
                    if (f.first >= Rx) break;
                    if (f.first > cur) { freeable.emplace_back(cur, min(f.first, Rx)); cur = min(Rx, f.second); }
                    else cur = max(cur, f.second);
                    if (cur >= Rx) break;
                }
                if (cur < Rx) freeable.emplace_back(cur, Rx);
            }
            freeable = merge_intervals(freeable);
            // only offload as much as needed (don't remove whole freeable segments if not required)
            // prioritize freeable segments that do NOT overlap the group gi+2 requests
            vector<pair<int,int>> next2_req;
            if (gi + 2 < groups.size()) next2_req = group_reqs[gi+2];
            auto overlaps_next2 = [&](const pair<int,int>& s)->bool{
                for (auto &r : next2_req) if (!(s.second <= r.first || s.first >= r.second)) return true;
                return false;
            };
            struct Item { pair<int,int> seg; int pr; };
            vector<Item> items;
            items.reserve(freeable.size());
            for (auto &seg : freeable) items.push_back({seg, overlaps_next2(seg) ? 1 : 0});
            stable_sort(items.begin(), items.end(), [](const Item &x,const Item &y){ return x.pr < y.pr; });
            long long need_free = max(0LL, need_bytes + cur_loaded - (long long)M);
            for (auto &it : items) {
                if (need_free <= 0) break;
                int Lx = it.seg.first, Rx = it.seg.second;
                int len = Rx - Lx;
                if (len <= 0) continue;
                int take = (int)min<long long>(len, need_free);
                int offL = Lx;
                int offR = Lx + take;
                out.push_back("Offload " + to_string(rw_time) + " " + to_string(offL) + " " + to_string(take));
                rw_time += 40LL * take;
                remove_interval(offL, offR);
                cur_loaded -= take;
                need_free -= take;
            }
            if (DEBUG) {
                cerr << "  after PhaseA offloads:\n";
                printLoaded(loaded, rw_time, comp_time, cur_loaded, M);
            }
        }

        // If still not enough, wait until previous group's visits finish (serialize visits), then offload again
        if (need_bytes + cur_loaded > M) {
            if (rw_time < comp_time) rw_time = comp_time; // allow offloads after visits complete
            auto forbidden2 = make_forbidden(rw_time); // now fewer protections
            vector<pair<int,int>> freeable2;
            for (auto &seg : loaded) {
                int Lx = seg.first, Rx = seg.second, cur = Lx;
                for (auto &f : forbidden2) {
                    if (f.second <= cur) continue;
                    if (f.first >= Rx) break;
                    if (f.first > cur) { freeable2.emplace_back(cur, min(f.first, Rx)); cur = min(Rx, f.second); }
                    else cur = max(cur, f.second);
                    if (cur >= Rx) break;
                }
                if (cur < Rx) freeable2.emplace_back(cur, Rx);
            }
            if (DEBUG) {
                cerr << "  after PhaseB offloads:\n";
                printLoaded(loaded, rw_time, comp_time, cur_loaded, M);
            }
            freeable2 = merge_intervals(freeable2);
            // only offload minimal amount required
            vector<pair<int,int>> next2_req_b;
            if (gi + 2 < groups.size()) next2_req_b = group_reqs[gi+2];
            auto overlaps_next2_b = [&](const pair<int,int>& s)->bool{
                for (auto &r : next2_req_b) if (!(s.second <= r.first || s.first >= r.second)) return true;
                return false;
            };
            struct Item2 { pair<int,int> seg; int pr; };
            vector<Item2> items2;
            items2.reserve(freeable2.size());
            for (auto &seg : freeable2) items2.push_back({seg, overlaps_next2_b(seg) ? 1 : 0});
            stable_sort(items2.begin(), items2.end(), [](const Item2 &x,const Item2 &y){ return x.pr < y.pr; });
            long long need_free2 = max(0LL, need_bytes + cur_loaded - (long long)M);
            for (auto &it : items2) {
                if (need_free2 <= 0) break;
                int Lx = it.seg.first, Rx = it.seg.second;
                int len = Rx - Lx;
                if (len <= 0) continue;
                int take = (int)min<long long>(len, need_free2);
                int offL = Lx;
                int offR = Lx + take;
                out.push_back("Offload " + to_string(rw_time) + " " + to_string(offL) + " " + to_string(take));
                rw_time += 40LL * take;
                remove_interval(offL, offR);
                cur_loaded -= take;
                need_free2 -= take;
            }
        }

        // As a last resort (very rare), offload smallest remaining loaded whole segments (but still avoid currently protected)
        if (need_bytes + cur_loaded > M) {
            vector<pair<int,int>> snap = loaded; // whole segments
            sort(snap.begin(), snap.end(), [](const pair<int,int>& x,const pair<int,int>& y){
                return (x.second-x.first) < (y.second-y.first);
            });
            auto forbidden_now = make_forbidden(rw_time);
            for (auto seg : snap) {
                long long need_free3 = max(0LL, need_bytes + cur_loaded - (long long)M);
                if (need_free3 <= 0) break;
                // skip if overlaps forbidden
                bool ok = true;
                for (auto &f : forbidden_now) if (!(seg.second <= f.first || seg.first >= f.second)) { ok = false; break; }
                if (!ok) continue;
                // prefer segments that do NOT overlap gi+2
                bool ov2 = false;
                if (gi + 2 < groups.size()) {
                    for (auto &r : group_reqs[gi+2]) if (!(seg.second <= r.first || seg.first >= r.second)) { ov2 = true; break; }
                }
                if (ov2) {
                    // postpone segments overlapping gi+2: push to back by re-adding at end
                    snap.push_back(seg);
                    continue;
                }
                int Lx = seg.first, Rx = seg.second; int len = Rx - Lx;
                if (len <= 0) continue;
                int take = (int)min<long long>(len, need_free3);
                int offL = Lx;
                int offR = Lx + take;
                out.push_back("Offload " + to_string(rw_time) + " " + to_string(offL) + " " + to_string(take));
                rw_time += 40LL * take;
                remove_interval(offL, offR);
                cur_loaded -= take;
            }
        }

        // schedule reloads for needed parts
        need = needed_parts(req); // recompute after offloads
        // record rw_time before loading group's required parts
        long long rw_before_group_load = rw_time;
        for (auto &p : need) {
            int Lx = p.first, Rx = p.second; int len = Rx - Lx;
            if (len <= 0) continue;
            out.push_back("Reload " + to_string(rw_time) + " " + to_string(Lx) + " " + to_string(len));
            rw_time += 40LL * len;
            merge_insert(Lx, Rx);
       }
        // rw_time after loading required parts
        long long group_rw_end = rw_time;
        if (DEBUG) {
            cerr << "  after required reloads (group_rw_end=" << group_rw_end << "):\n";
            printLoaded(loaded, rw_time, comp_time, total_loaded(), M);
        }
        // ---------- Prefetch next group's accesses ----------
        // Try to prefetch whole accesses from the next group in input order.
        // Only prefetch an access if ALL of its missing parts fit into current free HBM.
        if (gi + 1 < groups.size()) {
            long long free_bytes = M - total_loaded();
            if (DEBUG) cerr << "  prefetch: next_group=" << (gi+1) << " free_bytes=" << free_bytes << "\n";
            for (int idx : groups[gi+1]) {
                vector<pair<int,int>> single_req = { {a[idx].addr, a[idx].addr + a[idx].size} };
                auto need_next = needed_parts(single_req);
                long long need_next_bytes = 0;
                for (auto &q : need_next) need_next_bytes += (q.second - q.first);
                if (DEBUG) {
                    cerr << "    next idx=" << idx << " need_bytes=" << need_next_bytes;
                    printIntervals("    next need intervals", need_next);
                }
                if (need_next_bytes == 0) continue; // already fully loaded
                if (need_next_bytes <= free_bytes) {
                    // prefetch all missing parts for this access
                    for (auto &q : need_next) {
                        int Lp = q.first, Rp = q.second, len = Rp - Lp;
                        if (len <= 0) continue;
                        out.push_back("Reload " + to_string(rw_time) + " " + to_string(Lp) + " " + to_string(len));
                        rw_time += 40LL * len;
                        merge_insert(Lp, Rp);
                        free_bytes -= len;
                    }
                    if (DEBUG) {
                        cerr << "    prefetched idx=" << idx << " new_free_bytes=" << free_bytes << " rw_time=" << rw_time << "\n";
                        printLoaded(loaded, rw_time, comp_time, total_loaded(), M);
                    }
                } else {
                    if (DEBUG) cerr << "    skip prefetch idx=" << idx << " (need " << need_next_bytes << " > free " << free_bytes << ")\n";
                }
                if (free_bytes <= 0) break;
            }
        }
        // Visits must be serialized across groups: next group's visit cannot start before comp_time
        long long visit_start = max(comp_time, (long long)a[g.front()].start);
        // also cannot start before reloads finished
        visit_start = max(visit_start, group_rw_end);
        if (DEBUG) cerr << "  visit_start chosen = " << visit_start << " (comp_time=" << comp_time << ", group_rw_end=" << group_rw_end << ")\n";
        long long group_dur = 0;
        for (int idx : g) group_dur = max(group_dur, a[idx].time);
        for (int idx : g) out.push_back("Visit " + to_string(visit_start) + " " + to_string(idx));
        comp_time = visit_start + group_dur;
        group_end[gi] = comp_time;
        if (DEBUG) cerr << "Group " << gi << " scheduled visit at " << (comp_time - group_dur) << " dur=" << group_dur << " end=" << comp_time << "\n";
    }

    long long finish_time = max(rw_time, comp_time);
    out.push_back("Fin " + to_string(finish_time));
    // 按时间升序输出；同时间时按类型优先级：Visit < Reload < Offload < Fin ，
    // 同类型同时间保持生成顺序。
    struct Op { long long t; int pr; int idx; string line; };
    vector<Op> ops; ops.reserve(out.size());
    for (int i = 0; i < (int)out.size(); ++i) {
        const string &line = out[i];
        stringstream ss(line);
        string op; long long t = LLONG_MAX;
        if (ss >> op) {
            ss >> t; // all lines have time as second token (Fin also)
        }
        int pr = 4;
        if (op == "Visit") pr = 0;
        else if (op == "Reload") pr = 1;
        else if (op == "Offload") pr = 2;
        else if (op == "Fin") pr = 3;
        ops.push_back({t, pr, i, line});
    }
    sort(ops.begin(), ops.end(), [](const Op &a, const Op &b){
        if (a.t != b.t) return a.t < b.t;
        if (a.pr != b.pr) return a.pr < b.pr;
        return a.idx < b.idx;
    });
    for (auto &op : ops) cout << op.line << '\n';
    return 0;
}