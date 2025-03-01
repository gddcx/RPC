#ifndef _RANDOM_UMAP_H_
#define _RANDOM_UMAP_H_

#include <unordered_map>
#include <vector>

template <typename K, typename V>
struct Node {
    K key;
    V value;
    size_t index;
};

template <typename K, typename V>
class RandomUMap {
public:
    K RandomGetKey() {
        if(_vec.empty()) return K();
        size_t idx = std::rand() % _vec.size();
        return _vec[idx].key;
    }
    void Insert(K key, V value) {
        _umap[key] = value;
        _vec.push_back({key, value, _vec.size()});
        std::cout << __func__ << "key:" << key << " value:" << value << " size:" << _vec.size() << std::endl;
    }
    void Erase(K key) {
        if(_umap.find(key) != _umap.end()) {
            _vec.swap(_vec.back(), _vec[_umap[key].index]);
            _vec[_umap[key].index].index = _umap[key].index;
            _vec.pop_back();
            _umap.erase(key);
        }
    }
    typename std::unordered_map<K, V>::iterator find(K key) {
        return _umap.find(key);
    }
    typename std::unordered_map<K, V>::iterator end() {
        return _umap.end();
    }
    uint32_t size() {
        return _umap.size();
    }
    V& operator[](K key) {
        if(_umap.find(key) == _umap.end()) {
            Insert(key, V());
        }
        return _umap[key];
    }

private:
    std::unordered_map<K, V> _umap;
    std::vector<Node<K, V>> _vec;
};

#endif