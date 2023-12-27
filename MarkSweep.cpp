#include <iostream>
#include <vector>
#include <algorithm>
using namespace std;

template <class T>
struct GC_BTNode {
    //Normal BT Node members
    T info;
    GC_BTNode* left;
    GC_BTNode* right;
    //for GC use
    bool _gc_mark;
};

template <class T>
class GC_Allocator {
    private:
        vector<GC_BTNode<T>*> rootList;
        vector<GC_BTNode<T>*> nodeList;
        void mark_node(GC_BTNode<T>* node) {
            if (node != nullptr) {
                node->_gc_mark = true;
                mark_node(node->left);
                mark_node(node->right);
            }
        }
        void mark() {
            for (GC_BTNode<T>* x : rootList) {
                mark_node(x);
            }
        }
        void sweep() {
            int freedCnt = 0;
            vector<GC_BTNode<T>*> nextGen;
            for (auto& node : nodeList) {
                if (node->_gc_mark)
                    nextGen.push_back(std::move(node));
                else delete node;
            }
            freedCnt = nodeList.size() - nextGen.size();
            nodeList = nextGen;
            std::for_each(nodeList.begin(), nodeList.end(), [&](GC_BTNode<T>* h) { h->_gc_mark = false; });
            cout<<freedCnt<<" items freed."<<endl;
        }
    public:
        GC_Allocator() {

        }
        ~GC_Allocator() {
            while (!nodeList.empty()) {
                auto x = nodeList.back();
                nodeList.pop_back();
                delete x;
            }
        }
        GC_BTNode<T>* allocNode(T info, GC_BTNode<T>* left, GC_BTNode<T>* right) {
            GC_BTNode<T>* tmpNode = new GC_BTNode<T>;
            tmpNode->info = info;
            tmpNode->left = left;
            tmpNode->right = right;

            //all nodes start with mark set false.
            //and are immediately added to nodesList
            tmpNode->_gc_mark = false;
            nodeList.push_back(tmpNode);
            return tmpNode;
        }
        void addRoot(GC_BTNode<T>* root) {
            rootList.push_back(root);
        }
        void gc() {
            cout<<"[GC START]"<<endl;
            mark();
            sweep();
            cout<<"[GC END]"<<endl;
        }
};

template <class T>
class BST {
    private:
        GC_Allocator<T>* allocator;
        using nodeptr = GC_BTNode<T>*;
        nodeptr root;
        int count;
        nodeptr put(nodeptr h, T info) {
            if (h == nullptr) {
                count++;
                return allocator->allocNode(info, nullptr, nullptr);
            }
            if (info < h->info) h->left = put(h->left, info);
            else h->right = put(h->right, info);
            return h;
        }
        nodeptr erasemin(nodeptr h) {
            if (h->left == NULL)
                return h->right;
            h->left = erasemin(h->left);
            return h;
        }

        nodeptr min(nodeptr h) {
            nodeptr x = h;
            while (x->left)
                x = x->left;
            return x;
        }

        nodeptr erase(nodeptr h, T key) {
            if (h == NULL)
                return h;
            if (key < h->info)
                h->left = erase(h->left, key);
            else if (key > h->info)
                h->right = erase(h->right, key);
            else {
                if (h->left == NULL)
                    return h->right;
                if (h->right == NULL)
                    return h->left; 
                nodeptr t = h;
                h = min(t->right);
                h->right = erasemin(t->right);
                h->left = t->left;
                count--;
            }
            return h;
        }

        void show(nodeptr h) {
            if (h != NULL) {
                cout<<h->info<<" ";
                show(h->left);
                show(h->right);
            }
        }

    public:
        BST(GC_Allocator<T>* alloc) {
            allocator = alloc;
            root = nullptr;
        }
        void insert(T info) {
            if (root == nullptr) {
                root = allocator->allocNode(info, nullptr, nullptr);
                allocator->addRoot(root);
                count++;
            } else {
                root = put(root, info);
            }
        }
        void print() {
            show(root);
            cout<<endl;
        }
        void erase(T info) {
            root = erase(root, info);
        }
};

void test() {
    GC_Allocator<char> alloc;
    BST<char> bst(&alloc);
    string str = "GarbageCollectedBST";
    for (char c : str)
        bst.insert(c);
    bst.print();
    cout<<"Delete 'c'\nDelete 'S'"<<endl;
    bst.erase('c');
    bst.erase('S');
    bst.print();
    alloc.gc();
    bst.print();
    bst.insert('7');
    bst.print();
    bst.erase('C');
    alloc.gc();
    bst.print();
}


void test2() {
    GC_Allocator<int> alloc;
    BST<int> bst(&alloc);
    for (int i = 0; i < 50; i++) {
        bst.insert(rand() % 100);
    }
    for (int i = 5; i < 45; i++) {
        if (i % 2 == 0) {
            bst.erase(rand() % 100);
        }
        if (i % 7 == 0)
            alloc.gc();
    }
}

int main() {
    test();
    test2();
    return 0;
}