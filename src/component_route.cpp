//
// Created by MurphySL on 2020/10/23.
//

#include "weavess/component.h"

namespace weavess {
    /**
     * 贪婪搜索
     * @param query 查询点
     * @param pool 侯选池
     * @param res 结果集
     */
    void ComponentSearchRouteGreedy::RouteInner(unsigned int query, std::vector<Index::Neighbor> &pool, std::vector<unsigned int> &res) {
        const auto L = index->getParam().get<unsigned>("L_search");
        const auto K = index->getParam().get<unsigned>("K_search");

        std::vector<char> flags(index->getBaseLen());

        int k = 0;
        while (k < (int) L) {
            int nk = L;

            if (pool[k].flag) {
                pool[k].flag = false;
                unsigned n = pool[k].id;

                // 查找邻居的邻居
                for (unsigned m = 0; m < index->getFinalGraph()[n].size(); ++m) {
                    unsigned id = index->getFinalGraph()[n][m].id;

                    if (flags[id])continue;
                    flags[id] = 1;

                    float dist = index->getDist()->compare(index->getQueryData() + index->getQueryDim() * query,
                                                           index->getBaseData() + index->getBaseDim() * id,
                                                           (unsigned) index->getBaseDim());
                    index->addDistCount();

                    if (dist >= pool[L - 1].distance) continue;
                    Index::Neighbor nn(id, dist, true);
                    int r = Index::InsertIntoPool(pool.data(), L, nn);

                    //if(L+1 < retset.size()) ++L;
                    if (r < nk)nk = r;
                }
                //lock to here
            }
            if (nk <= k)k = nk;
            else ++k;
        }

        for (size_t i = 0; i < K; i++) {
            res[i] = pool[i].id;
        }
    }
}