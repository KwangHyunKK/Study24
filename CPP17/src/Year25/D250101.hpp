#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <algorithm>
#include <thread>
#include <iomanip>

namespace Y25M1
{
    /// #1. Fenwick Tree = Binary Indexed Tree(BIT)

    // index : 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    // value : 1 2 1 4 1 2 1 8 1 2  1  1  4  1  2  16
    // BIT   : 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
    // You can learn more at this page : https://www.acmicpc.net/blog/view/21

    class FenwickTree
    {
    public:
        FenwickTree(const std::vector<long long>& origin)
            :_array(1, 0)
        {
            _array.insert(_array.begin() + 1, origin.begin(), origin.end()); // _array size is _size + 1, not _size
            _size = origin.size();
            _tree.resize(_size + 1, 0);
            for(int i=1;i<=_size;++i)update(i, _array[i]);
        }
        FenwickTree(int size) : _size(size), _array(size + 1), _tree(size + 1)
        {
            // To do : Initialize _array and _tree with random value
        }
        long long sum(int i)
        {
            long long ans = 0;
            while(i > 0)
            {
                ans += _tree[i];
                i -= (i & -i);
            }
            return ans;
        }

        void update(int i, long long diff)
        {
            while(i < _tree.size())
            {
                _tree[i] += diff;
                i += (i & -i);
            }
        }

        std::ostream& operator<<(std::ostream& os)
        {
            if(_array.size() == 0)
            {
                os << "_array size is zero.\n";
            }
            else
            {
                os << "origin array : ";
                for(const auto& a : _array)
                {
                    os << a << " ";
                }
                os << "\ntree : ";
                for(const auto& b : _tree)
                {
                    os << b << " ";
                }
                os << "\n";
            }
            return os;
        }

        std::vector<long long> getTree() const
        {
            return _tree;
        }
    private:
        std::vector<long long> _array;
        std::vector<long long> _tree;
        int _size;
    };
}