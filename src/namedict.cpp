#include "pocketpy/namedict.h"

namespace pkpy{

uint16_t _find_perfect_hash_seed(uint16_t capacity, const std::vector<StrName>& keys){
    if(keys.empty()) return kHashSeeds[0];
    static std::set<uint16_t> indices;
    indices.clear();
    std::pair<uint16_t, float> best_score = {kHashSeeds[0], 0.0f};
    const int kHashSeedsSize = sizeof(kHashSeeds) / sizeof(kHashSeeds[0]);
    for(int i=0; i<kHashSeedsSize; i++){
        indices.clear();
        for(auto key: keys){
            uint16_t index = _hash(key, capacity-1, kHashSeeds[i]);
            indices.insert(index);
        }
        float score = indices.size() / (float)keys.size();
        if(score > best_score.second) best_score = {kHashSeeds[i], score};
    }
    return best_score.first;
}

}   // namespace pkpy