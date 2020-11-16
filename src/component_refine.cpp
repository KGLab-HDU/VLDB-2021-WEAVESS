//
// Created by MurphySL on 2020/10/23.
//

#include "weavess/component.h"

namespace weavess {
    /**
     * KGraph Refine :
     *  PRUNE        : NAIVE
     */
    void ComponentRefineNNDescent::RefineInner() {
        SetConfigs();

        unsigned range = index->K;

        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * range];

        // PRUNE
        std::cout << "__PRUNE : NAIVE__" << std::endl;
        auto *b = new ComponentPruneNaive(index);

#ifdef PARALLEL
#pragma omp parallel
#endif
        {
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};
#ifdef PARALLEL
#pragma omp for schedule(dynamic, 100)
#endif
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                b->PruneInner(n, range, flags, index->getFinalGraph()[n], cut_graph_);
            }
        }

#ifdef PARALLEL
#pragma omp for schedule(dynamic, 100)
#endif
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            Index::SimpleNeighbor *src_pool = cut_graph_ + n * range;
            int len = 0;
            for (unsigned i = 0; i < range; i++) {
                if (src_pool[i].distance == -1) break;
                len++;
                index->getFinalGraph()[n][i] = src_pool[i];
            }
            index->getFinalGraph()[n].resize(len);
        }

        delete[] cut_graph_;
    }

    void ComponentRefineNNDescent::SetConfigs() {
        index->K = index->getParam().get<unsigned>("K");
    }


    /**
     * FANNG Refine :
     *  PRUNE       : RNG
     */
    void ComponentRefineFANNG::RefineInner() {

        SetConfigs();

        unsigned range = index->R_refine;

        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * range];

        // PRUNE
        std::cout << "__PRUNE : RNG__" << std::endl;
        auto *b = new ComponentPruneHeuristic(index);

#ifdef PARALLEL
#pragma omp parallel
#endif
        {
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};
#ifdef PARALLEL
#pragma omp for schedule(dynamic, 100)
#endif
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {

                b->PruneInner(n, range, flags, index->getFinalGraph()[n], cut_graph_);
            }
        }

#ifdef PARALLEL
#pragma omp for schedule(dynamic, 100)
#endif
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            Index::SimpleNeighbor *src_pool = cut_graph_ + n * range;
            int len = 0;
            for (unsigned i = 0; i < range; i++) {
                if (src_pool[i].distance == -1) break;
                len++;
                index->getFinalGraph()[n][i] = src_pool[i];
            }
            index->getFinalGraph()[n].resize(len);
        }

        delete[] cut_graph_;
    }

    void ComponentRefineFANNG::SetConfigs() {
        index->R_refine = index->getParam().get<unsigned>("R_refine");
        index->M = index->getParam().get<unsigned>("M");
    }


    /**
    * NSG Refine :
    *  Entry     : Centroid
    *  CANDIDATE : GREEDY(NSG)
    *  PRUNE     : NSG
    *  CONN      : DFS
    */
    void ComponentRefineNSG::RefineInner() {

        SetConfigs();

        // ENTRY
        std::cout << "__ENTRY : Centroid__" << std::endl;
        auto *a = new ComponentRefineEntryCentroid(index);
        a->EntryInner();
        std::cout << "__ENTRY : FINISH" << std::endl;

        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->R_refine];
        Link(cut_graph_);

        index->getFinalGraph().resize(index->getBaseLen());

        for (size_t i = 0; i < index->getBaseLen(); i++) {
            Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->R_refine;
            unsigned pool_size = 0;
            for (unsigned j = 0; j < index->R_refine; j++) {
                if (pool[j].distance == -1) break;
                pool_size = j;
            }
            pool_size++;
            index->getFinalGraph()[i].resize(pool_size);
            for (unsigned j = 0; j < pool_size; j++) {
                index->getFinalGraph()[i][j].id = pool[j].id;
                index->getFinalGraph()[i][j].distance = pool[j].distance;
            }
        }

        // CONN
        std::cout << "__CONN : DFS__" << std::endl;
        auto *c = new ComponentConnNSGDFS(index);

        c->ConnInner();
    }

    void ComponentRefineNSG::SetConfigs() {
        index->R_refine = index->getParam().get<unsigned>("R_refine");
        index->L_refine = index->getParam().get<unsigned>("L_refine");
        index->C_refine = index->getParam().get<unsigned>("C_refine");

        index->width = index->R_refine;
    }

    void ComponentRefineNSG::Link(Index::SimpleNeighbor *cut_graph_) {
        std::vector<std::mutex> locks(index->getBaseLen());

        // CANDIDATE
        std::cout << "__CANDIDATE : GREEDY(NSG)__" << std::endl;
        auto *a = new ComponentCandidateNSG(index);

        // PRUNE
        std::cout << "__PRUNE : NSG__" << std::endl;
        auto *b = new ComponentPruneNSG(index);

#pragma omp parallel
        {
            std::vector<Index::SimpleNeighbor> pool;
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};

#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                pool.clear();
                flags.reset();

                a->CandidateInner(n, index->ep_, flags, pool);
                //std::cout << n << " candidate : " << pool.size() << std::endl;
                b->PruneInner(n, index->R_refine, flags, pool, cut_graph_);
                //std::cout << n << " prune : " << pool.size() << std::endl;
            }

            std::vector<Index::SimpleNeighbor>().swap(pool);
        }

#pragma omp for schedule(dynamic, 100)
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            InterInsert(n, index->R_refine, locks, cut_graph_);
        }
    }

    void ComponentRefineNSG::InterInsert(unsigned n, unsigned range, std::vector<std::mutex> &locks,
                                         Index::SimpleNeighbor *cut_graph_) {
        Index::SimpleNeighbor *src_pool = cut_graph_ + (size_t) n * (size_t) range;
        for (size_t i = 0; i < range; i++) {
            if (src_pool[i].distance == -1) break;

            Index::SimpleNeighbor sn(n, src_pool[i].distance);
            size_t des = src_pool[i].id;
            Index::SimpleNeighbor *des_pool = cut_graph_ + des * (size_t) range;

            std::vector<Index::SimpleNeighbor> temp_pool;
            int dup = 0;
            {
                Index::LockGuard guard(locks[des]);
                for (size_t j = 0; j < range; j++) {
                    if (des_pool[j].distance == -1) break;
                    if (n == des_pool[j].id) {
                        dup = 1;
                        break;
                    }
                    temp_pool.push_back(des_pool[j]);
                }
            }
            if (dup) continue;

            temp_pool.push_back(sn);
            if (temp_pool.size() > range) {
                std::vector<Index::SimpleNeighbor> result;
                unsigned start = 0;
                std::sort(temp_pool.begin(), temp_pool.end());
                result.push_back(temp_pool[start]);
                while (result.size() < range && (++start) < temp_pool.size()) {
                    auto &p = temp_pool[start];
                    bool occlude = false;
                    for (unsigned t = 0; t < result.size(); t++) {
                        if (p.id == result[t].id) {
                            occlude = true;
                            break;
                        }
                        float djk = index->getDist()->compare(
                                index->getBaseData() + index->getBaseDim() * (size_t) result[t].id,
                                index->getBaseData() + index->getBaseDim() * (size_t) p.id,
                                (unsigned) index->getBaseDim());
                        if (djk < p.distance /* dik */) {
                            occlude = true;
                            break;
                        }
                    }
                    if (!occlude) result.push_back(p);
                }
                {
                    Index::LockGuard guard(locks[des]);
                    for (unsigned t = 0; t < result.size(); t++) {
                        des_pool[t] = result[t];
                    }
                }
            } else {
                Index::LockGuard guard(locks[des]);
                for (unsigned t = 0; t < range; t++) {
                    if (des_pool[t].distance == -1) {
                        des_pool[t] = sn;
                        if (t + 1 < range) des_pool[t + 1].distance = -1;
                        break;
                    }
                }
            }
        }
    }

    /**
     * NSSG Refine :
     *  Entry      : Centroid
     *  CANDIDATE  : PROPAGATION 2
     *  PRUNE      : NSSG
     *  CONN       : DFS_Expand
     */
    void ComponentRefineSSG::RefineInner() {
        SetConfigs();

        // ENTRY
        auto *a = new ComponentRefineEntryCentroid(index);
        a->EntryInner();

        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->R_refine];
        Link(cut_graph_);

        index->getFinalGraph().resize(index->getBaseLen());

        for (size_t i = 0; i < index->getBaseLen(); i++) {
            Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->R_refine;
            unsigned pool_size = 0;
            for (unsigned j = 0; j < index->R_refine; j++) {
                if (pool[j].distance == -1) break;
                pool_size = j;
            }
            pool_size++;
            index->getFinalGraph()[i].resize(pool_size);
            for (unsigned j = 0; j < pool_size; j++) {
                Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                index->getFinalGraph()[i][j] = nn;
            }
        }

        // CONN
        std::cout << "__CONN : DFS__" << std::endl;
        auto *c = new ComponentConnSSGDFS(index);

        c->ConnInner();
    }

    void ComponentRefineSSG::SetConfigs() {
        index->R_refine = index->getParam().get<unsigned>("R_refine");
        index->L_refine = index->getParam().get<unsigned>("L_refine");
        index->A = index->getParam().get<float>("A");
        index->n_try = index->getParam().get<unsigned>("n_try");

        index->width = index->R_refine;
    }

    void ComponentRefineSSG::Link(Index::SimpleNeighbor *cut_graph_) {
        /*
         std::cerr << "Graph Link" << std::endl;
         unsigned progress = 0;
         unsigned percent = 100;
         unsigned step_size = nd_ / percent;
         std::mutex progress_lock;
         */
        std::vector<std::mutex> locks(index->getBaseLen());

        // CANDIDATE
        std::cout << "__CANDIDATE : PROPAGATION 2__" << std::endl;
        ComponentCandidate *a = new ComponentCandidatePropagation2(index);

        // PRUNE
        std::cout << "__PRUNE : NSSG__" << std::endl;
        ComponentPrune *b = new ComponentPruneSSG(index);

#pragma omp parallel
        {
            // unsigned cnt = 0;
            std::vector<Index::SimpleNeighbor> pool;
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};

#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                pool.clear();
                flags.reset();

                a->CandidateInner(n, n, flags, pool);
                //std::cout << "candidate : " << pool.size() << std::endl;

                b->PruneInner(n, index->R_refine, flags, pool, cut_graph_);
                //std::cout << "prune : " << pool.size() << std::endl;

                /*
                cnt++;
                if (cnt % step_size == 0) {
                  LockGuard g(progress_lock);
                  std::cout << progress++ << "/" << percent << " completed" << std::endl;
                }
                */
            }
        }

        double kPi = std::acos(-1);
        float threshold = std::cos(index->A / 180 * kPi);

#pragma omp for schedule(dynamic, 100)
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            InterInsert(n, index->R_refine, threshold, locks, cut_graph_);
        }
    }

    void ComponentRefineSSG::InterInsert(unsigned n, unsigned range, float threshold, std::vector<std::mutex> &locks,
                                         Index::SimpleNeighbor *cut_graph_) {
        Index::SimpleNeighbor *src_pool = cut_graph_ + (size_t) n * (size_t) range;
        for (size_t i = 0; i < range; i++) {
            if (src_pool[i].distance == -1) break;

            Index::SimpleNeighbor sn(n, src_pool[i].distance);
            size_t des = src_pool[i].id;
            Index::SimpleNeighbor *des_pool = cut_graph_ + des * (size_t) range;

            std::vector<Index::SimpleNeighbor> temp_pool;
            int dup = 0;
            {
                Index::LockGuard guard(locks[des]);
                for (size_t j = 0; j < range; j++) {
                    if (des_pool[j].distance == -1) break;
                    if (n == des_pool[j].id) {
                        dup = 1;
                        break;
                    }
                    temp_pool.push_back(des_pool[j]);
                }
            }
            if (dup) continue;

            temp_pool.push_back(sn);
            if (temp_pool.size() > range) {
                std::vector<Index::SimpleNeighbor> result;
                unsigned start = 0;
                std::sort(temp_pool.begin(), temp_pool.end());
                result.push_back(temp_pool[start]);
                while (result.size() < range && (++start) < temp_pool.size()) {
                    auto &p = temp_pool[start];
                    bool occlude = false;
                    for (unsigned t = 0; t < result.size(); t++) {
                        if (p.id == result[t].id) {
                            occlude = true;
                            break;
                        }
                        float djk = index->getDist()->compare(
                                index->getBaseData() + index->getBaseDim() * (size_t) result[t].id,
                                index->getBaseData() + index->getBaseDim() * (size_t) p.id,
                                (unsigned) index->getBaseDim());
                        float cos_ij = (p.distance + result[t].distance - djk) / 2 /
                                       sqrt(p.distance * result[t].distance);
                        if (cos_ij > threshold) {
                            occlude = true;
                            break;
                        }
                    }
                    if (!occlude) result.push_back(p);
                }
                {
                    Index::LockGuard guard(locks[des]);
                    for (unsigned t = 0; t < result.size(); t++) {
                        des_pool[t] = result[t];
                    }
                    if (result.size() < range) {
                        des_pool[result.size()].distance = -1;
                    }
                }
            } else {
                Index::LockGuard guard(locks[des]);
                for (unsigned t = 0; t < range; t++) {
                    if (des_pool[t].distance == -1) {
                        des_pool[t] = sn;
                        if (t + 1 < range) des_pool[t + 1].distance = -1;
                        break;
                    }
                }
            }
        }
    }

    /**
     * DPG Refine :
     *  Entry     : Centroid
     *  CANDIDATE : PROPAGATION 2
     *  PRUNE     : NSSG
     *  CONN      : DFS_Expand
     */
    void ComponentRefineDPG::RefineInner() {
        SetConfigs();
        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->L_dpg];
        Link(cut_graph_);

        index->getFinalGraph().resize(index->getBaseLen());

        for (size_t i = 0; i < index->getBaseLen(); i++) {
            Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->L_dpg;
            unsigned pool_size = 0;
            for (unsigned j = 0; j < index->L_dpg; j++) {
                if (pool[j].distance == -1) break;
                pool_size = j;
            }
            pool_size++;
            index->getFinalGraph()[i].resize(pool_size);
            for (unsigned j = 0; j < pool_size; j++) {
                Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                index->getFinalGraph()[i][j] = nn;
            }
        }

        // CONN
        std::cout << "__CONN : DFS__" << std::endl;
        auto *c = new ComponentConnReverse(index);
        c->ConnInner();
    }

    void ComponentRefineDPG::SetConfigs() {
        index->K = index->getParam().get<unsigned>("K");
        index->L = index->getParam().get<unsigned>("L");
        index->S = index->getParam().get<unsigned>("S");
        index->R = index->getParam().get<unsigned>("R");
        index->ITER = index->getParam().get<unsigned>("ITER");

        index->L_dpg = index->K / 2;
    }

    void ComponentRefineDPG::Link(Index::SimpleNeighbor *cut_graph_) {
        std::vector<std::mutex> locks(index->getBaseLen());

        // PRUNE
        ComponentPrune *b = new ComponentPruneDPG(index);

#pragma omp parallel
        {
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};
#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                //std::cout << n << std::endl;

                flags.reset();

                b->PruneInner(n, index->L_dpg, flags, index->getFinalGraph()[n], cut_graph_);
            }
        }
    }


    /**
     * VAMANA Refine :
     *  Entry        : Centroid
     *  CANDIDATE    : NSG
     *  PRUNE1       : Heuristic
     *  PRUNE2       : VAMANA
     */
    void ComponentRefineVAMANA::RefineInner() {
        SetConfigs();

        // ENTRY
        auto *a = new ComponentRefineEntryCentroid(index);
        a->EntryInner();

        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->R_refine];
        Link(cut_graph_);

        index->getFinalGraph().resize(index->getBaseLen());

        for (size_t i = 0; i < index->getBaseLen(); i++) {
            Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->R_refine;
            unsigned pool_size = 0;
            for (unsigned j = 0; j < index->R_refine; j++) {
                if (pool[j].distance == -1) break;
                pool_size = j;
            }
            pool_size++;
            index->getFinalGraph()[i].resize(pool_size);
            for (unsigned j = 0; j < pool_size; j++) {
                Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                index->getFinalGraph()[i][j] = nn;
            }
            std::sort(index->getFinalGraph()[i].begin(), index->getFinalGraph()[i].end());
        }

//        for(int i = 0; i < 100; i ++) {
//            std::cout << index->getFinalGraph()[i].size() << " : ";
//            for(int j = 0; j < index->getFinalGraph()[i].size(); j ++) {
//                std::cout << index->getFinalGraph()[i][j].id << "|" << index->getFinalGraph()[i][j].distance << " ";
//            }
//            std::cout << std::endl;
//        }
    }

    void ComponentRefineVAMANA::SetConfigs() {
        index->R_refine = index->getParam().get<unsigned>("R_refine");
    }

    void ComponentRefineVAMANA::Link(Index::SimpleNeighbor *cut_graph_) {
        std::vector<std::mutex> locks(index->getBaseLen());

        std::cout << "alpha " << index->alpha << std::endl;

        // CANDIDATE
        std::cout << "__CANDIDATE : NSG__" << std::endl;
        ComponentCandidate *a = new ComponentCandidateNSG(index);

        // PRUNE
        std::cout << "__PRUNE : VAMANA__" << std::endl;
        ComponentPrune *b = new ComponentPruneVAMANA(index);

#pragma omp parallel
        {
            std::vector<Index::SimpleNeighbor> pool;
            pool.resize(index->getBaseLen());
            boost::dynamic_bitset<> flags(index->getBaseLen(), 0);

#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                pool.clear();
                flags.reset();
                a->CandidateInner(n, index->ep_, flags, pool);

                b->PruneInner(n, index->R_refine, flags, pool, cut_graph_);
            }
        }

#pragma omp for schedule(dynamic, 100)
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            InterInsert(n, index->R_refine, locks, cut_graph_);
        }

        // set step 2 alpha
        index->alpha = 2;
    }

    void ComponentRefineVAMANA::InterInsert(unsigned int n, unsigned int range, std::vector<std::mutex> &locks,
                                            Index::SimpleNeighbor *cut_graph_) {
        // 结点 n 近邻
        Index::SimpleNeighbor *src_pool = cut_graph_ + (size_t) n * (size_t) range;
        // 遍历近邻
        for (size_t i = 0; i < range; i++) {
            if (src_pool[i].distance == -1) break;

            Index::SimpleNeighbor sn(n, src_pool[i].distance);
            size_t des = src_pool[i].id;
            // 结点 n 近 des 邻信息
            Index::SimpleNeighbor *des_pool = cut_graph_ + des * (size_t) range;

            // 单向边近邻 （n -> des.nn)
            std::vector<Index::SimpleNeighbor> temp_pool;
            // 查询 res 与 des近邻 是否存在双向边
            int dup = 0;
            {
                Index::LockGuard guard(locks[des]);
                for (size_t j = 0; j < range; j++) {
                    if (des_pool[j].distance == -1) break;
                    if (n == des_pool[j].id) {
                        dup = 1;
                        break;
                    }
                    temp_pool.push_back(des_pool[j]);
                }
            }
            if (dup) continue;

            // 插入待查询点 n
            temp_pool.push_back(sn);
            if (temp_pool.size() > range) {
                // 裁边
                std::vector<Index::SimpleNeighbor> result;
                Index::MinHeap<float, Index::SimpleNeighbor> skipped;
                std::sort(temp_pool.begin(), temp_pool.end());

                for (int k = temp_pool.size() - 1; k >= 0; --k) {
                    bool skip = false;
                    float cur_dist = temp_pool[i].distance;
                    for (size_t j = 0; j < result.size(); j++) {
                        float dist = index->getDist()->compare(
                                index->getBaseData() + index->getBaseDim() * (size_t) result[j].id,
                                index->getBaseData() + index->getBaseDim() * (size_t) temp_pool[k].id,
                                (unsigned) index->getBaseDim());
                        if (index->alpha * dist < cur_dist) {
                            skip = true;
                            break;
                        }
                    }

                    if (!skip) {
                        result.push_back(temp_pool[k]);
                    } else {
                        skipped.push(cur_dist, temp_pool[k]);
                    }

                    if (result.size() == range)
                        break;
                }

                while (result.size() < range && skipped.size()) {
                    result.push_back(skipped.top().data);
                    skipped.pop();
                }
                {
                    Index::LockGuard guard(locks[des]);
                    for (unsigned t = 0; t < result.size(); t++) {
                        des_pool[t] = result[t];
                    }
                }
            } else {
                // 添加反向边
                Index::LockGuard guard(locks[des]);
                for (unsigned t = 0; t < range; t++) {
                    if (des_pool[t].distance == -1) {
                        des_pool[t] = sn;
                        if (t + 1 < range) des_pool[t + 1].distance = -1;
                        break;
                    }
                }
            }
        }
    }


    /**
     * EFANNA Refine :
     *  Entry        : Rand
     *  CANDIDATE    : PROPAGATION 1
     *  PRUNE        : Naive
     */
    void ComponentRefineEFANNA::RefineInner() {
        SetConfigs();

        init();

        NNDescent();

        index->getFinalGraph().reserve(index->getBaseLen());

        for (unsigned i = 0; i < index->getBaseLen(); i++) {
            std::vector<Index::SimpleNeighbor> tmp;

            std::sort(index->graph_[i].pool.begin(), index->graph_[i].pool.end());

            for (auto &j : index->graph_[i].pool)
                tmp.push_back(Index::SimpleNeighbor(j.id, j.distance));

            index->getFinalGraph()[i] = tmp;

            // 内存释放
            std::vector<Index::Neighbor>().swap(index->graph_[i].pool);
            std::vector<unsigned>().swap(index->graph_[i].nn_new);
            std::vector<unsigned>().swap(index->graph_[i].nn_old);
            std::vector<unsigned>().swap(index->graph_[i].rnn_new);
            std::vector<unsigned>().swap(index->graph_[i].rnn_new);
        }

//        for(int i = 0; i < index->getBaseLen(); i ++) {
//            for(int j = 0; j < index->getFinalGraph()[i].size(); j ++) {
//                std::cout << index->getFinalGraph()[i][j].id << "|" << index->getFinalGraph()[i][j].distance << " ";
//            }
//            std::cout << std::endl;
//        }

        std::vector<Index::nhood>().swap(index->graph_);
    }

    void ComponentRefineEFANNA::SetConfigs() {
        index->L = index->getParam().get<unsigned>("L");
        index->K = index->getParam().get<unsigned>("K");
        index->R = index->getParam().get<unsigned>("R");
        index->S = index->getParam().get<unsigned>("S");
        index->ITER = index->getParam().get<unsigned>("ITER");
    }

    void ComponentRefineEFANNA::init() {
        index->graph_.reserve(index->getBaseLen());
        std::mt19937 rng(rand());

        for (unsigned i = 0; i < index->getBaseLen(); i++) {
            index->graph_.push_back(Index::nhood(index->L, index->S, rng, (unsigned) index->getBaseLen()));
        }

#pragma omp parallel for
        for (unsigned i = 0; i < index->getBaseLen(); i++) {
            auto &ids = index->getFinalGraph()[i];
            std::sort(ids.begin(), ids.end());

            for (unsigned j = 0; j < ids.size(); j++) {
                unsigned id = ids[j].id;
                if (id == i || (j > 0 && id == ids[j - 1].id)) continue;
                float dist = ids[j].distance;
                index->graph_[i].pool.push_back(Index::Neighbor(id, dist, true));
            }
            std::make_heap(index->graph_[i].pool.begin(), index->graph_[i].pool.end());
            index->graph_[i].pool.reserve(index->L);
        }
    }

    void ComponentRefineEFANNA::NNDescent() {

        std::mt19937 rng(rand());

        for (unsigned it = 0; it < index->ITER; it++) {
            join();
            update();
            //checkDup();
            std::cout << "iter: " << it << std::endl;
        }
    }

    void ComponentRefineEFANNA::join() {
#pragma omp parallel for default(shared) schedule(dynamic, 100)
        for (unsigned n = 0; n < index->getBaseLen(); n++) {
            index->graph_[n].join([&](unsigned i, unsigned j) {
                if (i != j) {
                    float dist = index->getDist()->compare(index->getBaseData() + i * index->getBaseDim(),
                                                           index->getBaseData() + j * index->getBaseDim(),
                                                           index->getBaseDim());

                    index->graph_[i].insert(j, dist);
                    index->graph_[j].insert(i, dist);
                }
            });
        }
    }

    void ComponentRefineEFANNA::update() {
        // 清空内存
#pragma omp parallel for
        for (unsigned i = 0; i < index->getBaseLen(); i++) {
            std::vector<unsigned>().swap(index->graph_[i].nn_new);
            std::vector<unsigned>().swap(index->graph_[i].nn_old);
            //std::vector<unsigned>().swap(graph_[i].rnn_new);
            //std::vector<unsigned>().swap(graph_[i].rnn_old);
            //graph_[i].nn_new.clear();
            //graph_[i].nn_old.clear();
            //graph_[i].rnn_new.clear();
            //graph_[i].rnn_old.clear();
        }
        // 确定候选个数
#pragma omp parallel for
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            auto &nn = index->graph_[n];
            std::sort(nn.pool.begin(), nn.pool.end());
            if (nn.pool.size() > index->L)nn.pool.resize(index->L);
            nn.pool.reserve(index->L);
            unsigned maxl = std::min(nn.M + index->S, (unsigned) nn.pool.size());
            unsigned c = 0;
            unsigned l = 0;
            //std::sort(nn.pool.begin(), nn.pool.end());
            //if(n==0)std::cout << nn.pool[0].distance<<","<< nn.pool[1].distance<<","<< nn.pool[2].distance<< std::endl;
            while ((l < maxl) && (c < index->S)) {
                if (nn.pool[l].flag) ++c;
                ++l;
            }
            nn.M = l;
        }
#pragma omp parallel for
        for (unsigned n = 0; n < index->getBaseLen(); ++n) {
            auto &nnhd = index->graph_[n];
            auto &nn_new = nnhd.nn_new;
            auto &nn_old = nnhd.nn_old;
            for (unsigned l = 0; l < nnhd.M; ++l) {
                auto &nn = nnhd.pool[l];
                auto &nhood_o = index->graph_[nn.id];  // nn on the other side of the edge

                if (nn.flag) {
                    nn_new.push_back(nn.id);
                    if (nn.distance > nhood_o.pool.back().distance) {
                        Index::LockGuard guard(nhood_o.lock);
                        if (nhood_o.rnn_new.size() < index->R)nhood_o.rnn_new.push_back(n);
                        else {
                            unsigned int pos = rand() % index->R;
                            nhood_o.rnn_new[pos] = n;
                        }
                    }
                    nn.flag = false;
                } else {
                    nn_old.push_back(nn.id);
                    if (nn.distance > nhood_o.pool.back().distance) {
                        Index::LockGuard guard(nhood_o.lock);
                        if (nhood_o.rnn_old.size() < index->R)nhood_o.rnn_old.push_back(n);
                        else {
                            unsigned int pos = rand() % index->R;
                            nhood_o.rnn_old[pos] = n;
                        }
                    }
                }
            }
            std::make_heap(nnhd.pool.begin(), nnhd.pool.end());
        }
#pragma omp parallel for
        for (unsigned i = 0; i < index->getBaseLen(); ++i) {
            auto &nn_new = index->graph_[i].nn_new;
            auto &nn_old = index->graph_[i].nn_old;
            auto &rnn_new = index->graph_[i].rnn_new;
            auto &rnn_old = index->graph_[i].rnn_old;
            if (index->R && rnn_new.size() > index->R) {
                std::random_shuffle(rnn_new.begin(), rnn_new.end());
                rnn_new.resize(index->R);
            }
            nn_new.insert(nn_new.end(), rnn_new.begin(), rnn_new.end());
            if (index->R && rnn_old.size() > index->R) {
                std::random_shuffle(rnn_old.begin(), rnn_old.end());
                rnn_old.resize(index->R);
            }
            nn_old.insert(nn_old.end(), rnn_old.begin(), rnn_old.end());
            if (nn_old.size() > index->R * 2) {
                nn_old.resize(index->R * 2);
                nn_old.reserve(index->R * 2);
            }
            std::vector<unsigned>().swap(index->graph_[i].rnn_new);
            std::vector<unsigned>().swap(index->graph_[i].rnn_old);
        }
    }


    /**
     * ONNG Refine :
     *  PRUNE      : ONNG
     */
    void ComponentRefineONNG::RefineInner() {

    }


    /**
     * SPTAG_BKT Refine :
     *
     */
    void ComponentRefineSPTAG_BKT::RefineInner() {
        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->m_iNeighborhoodSize];
        index->getFinalGraph().resize(index->getBaseLen());

        unsigned m_iRefineIter = 2;
        for (int iter = 0; iter < m_iRefineIter - 1; iter++) {
            index->L_refine = index->m_iCEF * index->m_iCEFScale;
            index->R_refine = index->m_iNeighborhoodSize;
            Link(cut_graph_);

            for (size_t i = 0; i < index->getBaseLen(); i++) {
                Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->m_iNeighborhoodSize;
                unsigned pool_size = 0;
                for (unsigned j = 0; j < index->m_iNeighborhoodSize; j++) {
                    if (pool[j].distance == -1) break;
                    pool_size = j;
                }
                pool_size++;
                index->getFinalGraph()[i].resize(pool_size);
                for (unsigned j = 0; j < pool_size; j++) {
                    Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                    index->getFinalGraph()[i][j] = nn;
                }
            }
        }

//        index->m_iNeighborhoodSize /= index->m_iNeighborhoodScale;  //K
//        auto *cut_graph2_ = new Index::SimpleNeighbor[index->getBaseLen() * index->m_iNeighborhoodSize];
//
//        if (m_iRefineIter > 0) {
//            index->L_refine = index->m_iCEF * index->m_iCEFScale;
//            index->R_refine = index->m_iNeighborhoodSize;
//            Link(cut_graph2_);
//
//            for (size_t i = 0; i < index->getBaseLen(); i++) {
//                Index::SimpleNeighbor *pool = cut_graph2_ + i * (size_t) index->m_iNeighborhoodSize;
//                unsigned pool_size = 0;
//                for (unsigned j = 0; j < index->m_iNeighborhoodSize; j++) {
//                    if (pool[j].distance == -1) break;
//                    pool_size = j;
//                }
//                //pool_size++;
//                index->getFinalGraph()[i].resize(pool_size);
//                for (unsigned j = 0; j < pool_size; j++) {
//                    Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
//                    index->getFinalGraph()[i][j] = nn;
//                }
//            }
//        }

    }

    void ComponentRefineSPTAG_BKT::Link(Index::SimpleNeighbor *cut_graph_) {
        std::vector<std::mutex> locks(index->getBaseLen());

        // CANDIDATE
        std::cout << "__CANDIDATE : SPTAG_BKT__" << std::endl;
        auto *a = new ComponentCandidateSPTAG_BKT(index);

        // PRUNE
        std::cout << "__PRUNE : NAIVE__" << std::endl;
        auto *b = new ComponentPruneNaive(index);

#pragma omp parallel
        {
            std::vector<Index::SimpleNeighbor> pool;
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};

#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                pool.clear();
                flags.reset();

                a->CandidateInner(n, index->ep_, flags, pool);
                //std::cout << n << " candidate : " << pool.size() << std::endl;
                b->PruneInner(n, index->R_refine, flags, pool, cut_graph_);
                //std::cout << n << " prune : " << pool.size() << " " << index->R_refine << std::endl;
            }

            std::vector<Index::SimpleNeighbor>().swap(pool);
        }
    }


    /**
     * SPTAG_KDT Refine :
     */
    void ComponentRefineSPTAG_KDT::RefineInner() {
        auto *cut_graph_ = new Index::SimpleNeighbor[index->getBaseLen() * (size_t) index->m_iNeighborhoodSize];
        index->getFinalGraph().resize(index->getBaseLen());

        unsigned m_iRefineIter = 2;
        for (int iter = 0; iter < m_iRefineIter - 1; iter++) {
            index->L_refine = index->m_iCEF * index->m_iCEFScale;
            index->R_refine = index->m_iNeighborhoodSize;
            Link(cut_graph_);

            for (size_t i = 0; i < index->getBaseLen(); i++) {
                Index::SimpleNeighbor *pool = cut_graph_ + i * (size_t) index->m_iNeighborhoodSize;
                unsigned pool_size = 0;
                for (unsigned j = 0; j < index->m_iNeighborhoodSize; j++) {
                    if (pool[j].distance == -1) break;
                    pool_size = j;
                }
                //pool_size++;
                index->getFinalGraph()[i].resize(pool_size);
                for (unsigned j = 0; j < pool_size; j++) {
                    Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                    index->getFinalGraph()[i][j] = nn;
                }
            }
        }

//        for(int i = 0; i < index->getBaseLen(); i ++) {
//            std::cout << i << " " << index->getFinalGraph()[i].size() << std::endl;
//            for(int j = 0; j < index->getFinalGraph()[i].size(); j ++) {
//                if(index->getFinalGraph()[i][j].id > index->getBaseLen()) {
//                    std::cout << "wtf" << std::endl;
//                    std::cout << i << " " << j << " " << index->getFinalGraph()[i][j].id << " " << index->getFinalGraph()[i][j].distance << std::endl;
//                }
//            }
//        }

        index->m_iNeighborhoodSize /= index->m_iNeighborhoodScale;  //K
        auto *cut_graph2_ = new Index::SimpleNeighbor[index->getBaseLen() * index->m_iNeighborhoodSize];

        if (m_iRefineIter > 0) {
            index->L_refine = index->m_iCEF;
            index->R_refine = index->m_iNeighborhoodSize;
            Link(cut_graph2_);

            for (size_t i = 0; i < index->getBaseLen(); i++) {
                Index::SimpleNeighbor *pool = cut_graph2_ + i * (size_t) index->m_iNeighborhoodSize;
                unsigned pool_size = 0;
                for (unsigned j = 0; j < index->m_iNeighborhoodSize; j++) {
                    if (pool[j].distance == -1) break;
                    pool_size = j;
                }
                //pool_size++;
                index->getFinalGraph()[i].resize(pool_size);
                for (unsigned j = 0; j < pool_size; j++) {
                    Index::SimpleNeighbor nn(pool[j].id, pool[j].distance);
                    index->getFinalGraph()[i][j] = nn;
                }
            }
        }
    }

    void ComponentRefineSPTAG_KDT::Link(Index::SimpleNeighbor *cut_graph_) {
        std::vector<std::mutex> locks(index->getBaseLen());

        // CANDIDATE
        std::cout << "__CANDIDATE : SPTAG_KDT__" << std::endl;
        auto *a = new ComponentCandidateSPTAG_KDT(index);

        // PRUNE
        std::cout << "__PRUNE : NAIVE__" << std::endl;
        auto *b = new ComponentPruneNaive(index);

#pragma omp parallel
        {
            std::vector<Index::SimpleNeighbor> pool;
            boost::dynamic_bitset<> flags{index->getBaseLen(), 0};

#pragma omp for schedule(dynamic, 100)
            for (unsigned n = 0; n < index->getBaseLen(); ++n) {
                std::cout << "prune " << n << std::endl;
                pool.clear();
                flags.reset();

                a->CandidateInner(n, n, flags, pool);
                //std::cout << "candidate finish" << std::endl;
                //std::cout << n << " candidate : " << pool.size() << std::endl;
                b->PruneInner(n, index->R_refine, flags, pool, cut_graph_);
                //std::cout << "prune finish" << std::endl;
                //std::cout << n << " prune : " << pool.size() << std::endl;
            }

            std::vector<Index::SimpleNeighbor>().swap(pool);
        }
    }
}