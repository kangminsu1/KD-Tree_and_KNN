#ifndef __HEADER_KDTREE_H__
#define __HEADER_KDTREE_H__

#include <vector>
#include <numeric>
#include <algorithm>
#include <functional>

namespace kd{
    template<class pointT>
    class kdtree{
        public:
            kdtree() : root(nullptr) {};
            kdtree(const std::vector<pointT>& point) : root(nullptr) { build(point); };
            ~kdtree() { clear();}

            void build(const std::vector<pointT>& point){
                clear();
                points = point;
                std::vector<int> indice(points.size());
                std::iota(std::begin(indice), std::end(indice), 0); // 모든 값 0으로 초기화

                //vector.data() == 첫 데이터의 주소값을 입력
                root = build_recursive(indice.data(), (int)point.size(), 0);
                // 각 원소들의 reference들을 정렬
            }

            void clear(){
                clear_recursive(root);
                root = nullptr;
                points.clear();
            }
            
            int nnsearch(const pointT& quary, double* mindist = nullptr) const {
                int guess;
                double _mindist = std::numeric_limits<double>::max();
                nnserach_recursive(quary, root, &guess, &_mindist);
                return guess;
            }

            std::vector<int> knnsearch(const pointT& query, int k) const {
                knn_query queue(k);
                knnSearchRecursive(query, root, queue, k);
                
			    std::vector<int> indices(queue.size());
                for (size_t i = 0; i < queue.size(); i++)
                    indices[i] = queue[i].second;

                return indices;

            }

        private:
            struct Node{
                int idx;
                Node* next[2];
                int axis;

                // node 선언할 때 초기 값
                Node() : idx(-1), axis(-1) {next[0] = next[1] = nullptr; } 
            };

            Node* root;
            std::vector<pointT> points;
            
            template <class T, class Compare = std::less<T>>
            class bound_priority_queue{
                public:
                    bound_priority_queue() = delete;
                    bound_priority_queue(size_t bound) : bound_(bound) { elements_.reserve(bound + 1); };

                    void push(const T& val)
                    {
                        auto it = std::find_if(std::begin(elements_), std::end(elements_),
                            [&](const T& element){ return Compare()(val, element); });
                        elements_.insert(it, val);

                        if (elements_.size() > bound_)
                            elements_.resize(bound_);
                    }

                    const T& back() const {return elements_.back();};
                    const T& operator[](size_t index) const {return elements_[index]; }
                    size_t size() const {return elements_.size(); }

                private:
                    size_t bound_;
                    std::vector<T> elements_;
            };
            using knn_query = bound_priority_queue<std::pair<double, int>>;

            Node* build_recursive(int* indice, int npoints, int depth){

                if(npoints <= 0) return nullptr;

                const int axis = depth % pointT::DIM;
                const int mid = (npoints - 1) / 2;

                std::nth_element(indice, indice + mid, indice + npoints, [&](int a, int b){
                        return points[a][axis] < points[b][axis];
                });
                
                Node* node = new Node();
                node->idx = indice[mid];
                node->axis = axis;
                
                node->next[0] = build_recursive(indice, mid, depth + 1);
                node->next[1] = build_recursive(indice + mid + 1, npoints - mid - 1, depth + 1);

                return node;
            }

            void clear_recursive(Node* root_node){
                if(root_node == nullptr) return;

                if(root_node->next[0]) clear_recursive(root_node->next[0]);
                if(root_node->next[1]) clear_recursive(root_node->next[1]);
                delete root_node;
            }

            static double distance(const pointT& p, const pointT& q){
                double dist = 0;
                for(size_t i = 0; i < pointT::DIM; i++){
                    dist += (p[i] - q[i]) * (p[i] - q[i]);
                }
                return sqrt(dist);
            }

            void nnserach_recursive(const pointT& query, const Node* node, int *guess, double *mindist) const {
                if(node == nullptr) return;
                const pointT& train = points[node->idx];
                const double dist = distance(query, train);
                if(dist < *mindist){
                    *mindist = dist;
                    *guess = node->idx;
                }

                const int axis = node->axis;
                const int dir = query[axis] < train[axis] ? 0 : 1;
                nnserach_recursive(query, node->next[dir], guess, mindist);
                const double diff = fabs(query[axis] - train[axis]);
                if(diff < *mindist) nnserach_recursive(query, node->next[!dir], guess, mindist);
            }

            void knnSearchRecursive(const pointT& query, const Node* node, knn_query& queue, int k) const
            {
                if (node == nullptr)
                    return;

                const pointT& train = points[node->idx];

                const double dist = distance(query, train); // 초기 root와 비교할 Target Point의 상대 거리
                queue.push(std::make_pair(dist, node->idx));

                const int axis = node->axis;
                const int dir = query[axis] < train[axis] ? 0 : 1;
                knnSearchRecursive(query, node->next[dir], queue, k);

                const double diff = fabs(query[axis] - train[axis]);
                if ((int)queue.size() < k || diff < queue.back().first)
                    knnSearchRecursive(query, node->next[!dir], queue, k);
            }
    };
}

#endif