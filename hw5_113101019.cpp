#include <iostream>
#include <cstdint>
#include <vector>
#include <iomanip>
#include <sys/time.h>
#include <queue>
#include <fstream>
using namespace std;

struct Cache {
    long long addr;   
    Cache *prev;
    Cache *next;
    bool place;
    bool dirty;
    Cache(long long a) : addr(a), prev(NULL), next(NULL),dirty(false) {}
};


struct HashNode {
    long long key;   
    Cache* value;   
    HashNode* next;
    HashNode(long long k, Cache* v) : key(k), value(v), next(NULL) {}
};

class HashTable {
private:
    static const int HASH_SIZE = 100003; 
    HashNode* table[HASH_SIZE];

    int hash_func(long long key) const {
        long long h = key % HASH_SIZE;
        if (h < 0) h += HASH_SIZE;
        return (int)h;
    }
public:
    HashTable() {
        for (int i = 0; i < HASH_SIZE; ++i) {
            table[i] = NULL;
        }
    }

    Cache* find(long long key) const {
        int h = hash_func(key);
        HashNode* cur = table[h];
        while (cur) {
            if (cur->key == key) return cur->value;
            cur = cur->next;
        }
        return NULL;
    }

    void insert(long long key, Cache* val) {
        int h = hash_func(key);
        HashNode* cur = table[h];

        while (cur) {
            if (cur->key == key) {
                cur->value = val;
                return;
            }
            cur = cur->next;
        }

        HashNode* node = new HashNode(key, val);
        node->next = table[h];
        table[h] = node;
    }

    void erase(long long key) {
        int h = hash_func(key);
        HashNode* cur = table[h];
        HashNode* prev = NULL;

        while (cur) {
            if (cur->key == key) {
                if (prev) prev->next = cur->next;
                else table[h] = cur->next;
                delete cur;
                return;
            }
            prev = cur;
            cur = cur->next;
        }
    }
};

void move_to_head(Cache *&p,Cache *&head,Cache *&tail)
{
    if(p == head)
    {
        return;
    }
    if(p==tail)
    {
        tail = tail->prev;
        if(tail)
        tail->next = NULL;
    }
    else
    {
        p->prev->next = p->next;
        p->next->prev = p->prev;
    }
    p->next = head; 
    p->prev = NULL; 
    
    if (head) {
        head->prev = p; 
    }
    head = p; 
}

int main(int argc, char* argv[]) {
    int space[5] = {4096, 8192, 16384, 32768, 65536};

    string op1;
    uint64_t byte_offset1;
    vector<pair<string,uint64_t> > all;
    ifstream infile(argv[1]);
    while (infile >> op1 >> std::hex >> byte_offset1)
    {
        pair<string,uint64_t> f;
        f.first = op1;
        f.second = byte_offset1;
        all.push_back(f);
    }
    infile.close();
    cout << "LRU policy:" << endl;
    cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n" ;
    struct timeval start, end;
    gettimeofday(&start, NULL);
    for(int i:space)
    {
        
        HashTable ht;           

        uint64_t hit = 0,byte_offset;
        uint64_t miss = 0;
        uint64_t wbc = 0;
        uint64_t total_count = 0;
        Cache* head = NULL;  
        Cache* tail = NULL;  
        string op;
        for(pair<string,uint64_t> k:all)
        {
            op=k.first;
            byte_offset = k.second;
            long long addr = byte_offset;  
            uint64_t page = byte_offset / 4096;
            Cache* node = ht.find((long long)page);
            if (node) 
            {
                hit++;
                if (op == "W") 
                {
                    node->dirty = true;
                }
                move_to_head(node,head,tail);
            } 
            else {
                miss++;
                Cache* newNode = new Cache(page);

                newNode->next = head;
                if (head) 
                {
                    head->prev = newNode;
                    head = newNode;
                }
                else
                head = newNode;
                if (tail == NULL) 
                tail = newNode;
                if (op == "W") 
                {
                    newNode->dirty = true;
                }
                ht.insert(page, newNode);
                if (total_count < i) 
                {
                    total_count++;
                }
                else
                {
                    Cache *p = tail;
                    if(p->dirty)
                    wbc++;
                    ht.erase(p->addr);
                    tail = tail->prev;
                    if (tail) 
                    {
                        tail->next = NULL;
                    } 
                    else 
                    {
                        
                        head = NULL; 
                    }                
                    delete(p);
                }
            }
        }
        cout << i << "\t" << hit << "\t"  << miss << "\t\t"  << fixed << setprecision(10) << (double)miss / all.size() << "\t\t"  << wbc << endl;    
    }
    gettimeofday(&end, NULL);
    
    double elapsed = (end.tv_sec - start.tv_sec) + 
                     (end.tv_usec - start.tv_usec) / 1000000.0;

    cout << "Elapsed time: " << fixed << setprecision(6) << elapsed << " sec" << endl<<endl;

    cout << "CFLRU policy:" << endl;
    cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\tWrite back count\n" ;
    struct timeval start1, end1;
    gettimeofday(&start1, NULL);
    for(int i:space)
    {
        
        HashTable ht1;           

        uint64_t hit1 = 0,byte_offset1;
        uint64_t miss1 = 0;
        uint64_t wbc1 = 0;
        uint64_t total_count1 = 0,clean_count=i/4;
        Cache* head1 = NULL;  
        Cache* tail1 = NULL;  
        string op1;
        for(pair<string,uint64_t> k:all)
        {
            op1=k.first;
            byte_offset1 = k.second;
            long long addr = byte_offset1;  
            uint64_t page = byte_offset1 / 4096;
            Cache* node = ht1.find((long long)page);
            if (node) 
            {
                hit1++;
                if (op1 == "W") 
                {
                    node->dirty = true;
                }
                move_to_head(node,head1,tail1);
            } 
            else {
                miss1++;
                Cache* newNode = new Cache(page);

                newNode->next = head1;
                if (head1) 
                {
                    head1->prev = newNode;
                    head1 = newNode;
                }
                else
                head1 = newNode;
                if (tail1 == NULL) 
                tail1 = newNode;
                if (op1 == "W") 
                {
                    newNode->dirty = true;
                }
                ht1.insert(page, newNode);
                if (total_count1 < i) 
                {
                    total_count1++;
                }
                else
                {
                    Cache* p = tail1;
                    int count=1;
                    while(p->dirty!=false && count < clean_count)
                    {
                        p = p->prev;
                        count++;
                    }
                    if(count == clean_count && p->dirty!=false)
                    {
                        p=tail1;
                        if(p->dirty)
                        wbc1++;
                        ht1.erase(p->addr);
                        tail1 = tail1->prev;
                        if (tail1) 
                        {
                            tail1->next = NULL;
                        } 
                        else 
                        {
                            head1 = NULL; 
                        }                
                        delete(p);
                    }
                    else
                    {
                        if(p==tail1)
                        {
                            ht1.erase(p->addr);
                            tail1 = tail1->prev;
                            if (tail1) 
                            {
                                tail1->next = NULL;
                            } 
                            else 
                            {
                                head1 = NULL; 
                            }                
                            delete(p);
                        }
                        else
                        {
                        p->prev->next=p->next;
                        p->next->prev = p->prev;
                        ht1.erase(p->addr);
                        delete(p);
                        }
                    }
                }
            }
        }
        cout << i << "\t" << hit1 << "\t"  << miss1 << "\t\t"  << fixed << setprecision(10) << (double)miss1 / all.size() << "\t\t"  << wbc1 << endl;    
    }
    gettimeofday(&end1, NULL);
    
    double elapsed1 = (end1.tv_sec - start1.tv_sec) + 
                     (end1.tv_usec - start1.tv_usec) / 1000000.0;

    cout << "Elapsed time: " << fixed << setprecision(6) << elapsed1 << " sec" << endl<<endl;
}