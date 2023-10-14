#include <vector>
#include <string>
#include <queue>
#include <iostream>
#include <algorithm>
#include <unordered_map>

typedef long long LL;

struct Node{
    std::unordered_map<int, Node*> children;
    bool terminal;

    Node(){
        terminal = false;
    }
};

struct Sieve{
    LL limit;
    std::vector<bool> prime;

    Sieve(LL limit){
        this->limit = limit;
        prime = std::vector<bool>(limit + 1, false);
    }

    std::vector<LL> to_list(){
        std::vector<LL> result;
        result.push_back(2);
        result.push_back(3);
        for(LL p = 5; p <= limit; p++){
            if(prime[p]){
                result.push_back(p);
            }
        }
        return result;
    }

    void omit_squares(){
        LL r = 5;
        while(r * r < limit){
            if(prime[r]){
                LL i = r * r;
                while(i < limit){
                    prime[i] = false;
                    i = i + r * r;
                }
            }
            r += 1;
        }
    }

    void step1(LL x, LL y){
        LL n = (4 * x * x) + (y * y);
        if(n <= limit && (n % 12 == 1 || n % 12 == 5)){
            prime[n] = !prime[n];
        }
    }

    void step2(LL x, LL y){
        LL n = (3 * x * x) + (y * y);
        if(n <= limit && n % 12 == 7){
            prime[n] = !prime[n];
        }
    }

    void step3(LL x, LL y){
        LL n = (3 * x * x) - (y * y);
        if(x > y && n <= limit && n % 12 == 11){
            prime[n] = !prime[n];
        }
    }

    void loop_y(LL x){
        LL y = 1;
        while(y * y < limit){
            step1(x, y);
            step2(x, y);
            step3(x, y);
            y += 1;
        }
    }

    void loop_x(){
        LL x = 1;
        while(x * x < limit){
            loop_y(x);
            x += 1;
        }
    }

    void calc(){
        loop_x();
        omit_squares();
    }
};

Node *generate_trie(std::vector<LL> l){
    Node *root = new Node();
    for(LL el : l){
        Node *head = root;
        std::string s = std::to_string(el);
        for(char ch : s){
            if(head->children.find(ch) == head->children.end()){
                head->children[ch] = new Node();
            }
            head = head->children[ch];
        }
        head->terminal = true;
    }
    return root;
}

std::vector<LL> find(LL upper_bound, LL prefix){
    Sieve *sieve = new Sieve(upper_bound);
    sieve->calc();
    std::string str_prefix = std::to_string(prefix);
    Node *head = generate_trie(sieve->to_list());
    for(char ch : str_prefix){
        if(head->children.find(ch) == head->children.end()){
            return std::vector<LL>();
        }
        head = head->children[ch];
    }

    std::queue<std::pair<Node*, std::string>> q;
    std::vector<LL> result;
    q.push(std::make_pair(head, str_prefix));
    while(!q.empty()){
        std::pair<Node*, std::string> top = q.front();
        q.pop();
        if(top.first->terminal){
            result.push_back(std::stoll(top.second));
        }
        for(std::pair<char, Node*> p : top.first->children){
            q.push(std::make_pair(p.second, top.second + p.first));
        }
    }

    std::sort(result.begin(), result.end());
    return result;
}

void verify(){
    std::vector<LL> left = {2, 23, 29};
    std::vector<LL> right = find(100, 2);
    if(left != right){
        std::cout << "left != right" << std::endl;
        exit(1);
    }
}


int main(){
    const LL UPPER_BOUND = 5000000;
    const LL PREFIX = 32338;

    verify();
    std::vector<LL> results = find(UPPER_BOUND, PREFIX);
    std::vector<LL> expected = {323381, 323383, 3233803, 3233809, 3233851, 3233863, 3233873, 3233887, 3233897};
    if(results != expected){
        std::cout << "results != expected" << std::endl;
        exit(1);
    }
    return 0;
}