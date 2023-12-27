#include <iostream>
using namespace std;

template <class T>
struct GC_BTNode {
    //Normal BT Node members
    T info;
    GC_BTNode* left;
    GC_BTNode* right;
    //for GC use
    bool _gc_mark;
    GC_BTNode* _gc_next;
};

template <class T>
struct GC_BT_Root {
    GC_BTNode<T>* head;
    GC_BT_Root* next;
};

template <class T>
class GC_Allocator {
    private:
        GC_BT_Root<T>* rootList;
        GC_BTNode<T>* nodeList;
        void mark_node(GC_BTNode<T>* node) {
            if (node != nullptr) {
                node->_gc_mark = true;
                mark_node(node->left);
                mark_node(node->right);
            }
        }
        void mark() {
            GC_BT_Root<T>* x = rootList;
            while (x != nullptr) {
                mark_node(x->head);
                x = x->next;
            }
        }
        void sweep() {
            int freedCnt = 0;
            GC_BTNode<T>* x = nodeList;
            GC_BTNode<T>* nextGen = nullptr;
            while (x != nullptr) {
                GC_BTNode<T>* nx = x->_gc_next;
                if (x->_gc_mark == false) {
                    //node couldn;t be reached so it is not in use, ok to free memory.
                    x->left = nullptr;
                    x->right = nullptr;
                    x->_gc_next = nullptr;
                    cout<<x->info<<" is unreachable, freeing memory."<<endl;
                    delete x;
                    freedCnt++;
                } else {
                    //node was reached during mark phase, so its not garbage.
                    //un mark it for next mark phase.
                    x->_gc_mark = false;
                    x->_gc_next = nextGen;
                    nextGen = x;
                }
                x = nx;
            }
            nodeList = nextGen;
            cout<<freedCnt<<" items freed."<<endl;
        }
    public:
        GC_Allocator() {
            rootList = nullptr;
            nodeList = nullptr;
        }
        GC_BTNode<T>* allocNode(T info, GC_BTNode<T>* left, GC_BTNode<T>* right) {
            GC_BTNode<T>* tmpNode = new GC_BTNode<T>;
            tmpNode->info = info;
            tmpNode->left = left;
            tmpNode->right = right;

            //all nodes start with mark set false.
            //and are immediately added to nodesList
            tmpNode->_gc_mark = false;
            tmpNode->_gc_next = nodeList;
            nodeList = tmpNode;
            return tmpNode;
        }
        void addRoot(GC_BTNode<T>* root) {
            GC_BT_Root<T>* tmpRoot = new GC_BT_Root<T>;
            tmpRoot->head = root;
            tmpRoot->next = rootList;
            rootList = tmpRoot; 
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

int main() {
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
    return 0;
}