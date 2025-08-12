#include <iostream>
#include <vector>
#include <string>
#include <unordered_set>
using namespace std;

struct TrieNode {
    TrieNode* children[26];
    unordered_set<int> strings; // 经过此节点的字符串编号
    
    TrieNode() {
        for (int i = 0; i < 26; i++) {
            children[i] = nullptr;
        }
    }
};

class Solution {
private:
    TrieNode* root;
    vector<string> names;
    vector<long long> ans;
    int n;
    
    void insertAllSuffixes(int stringId) {
        string& name = names[stringId];
        int len = static_cast<int>(name.length());
        
        for (int i = 0; i < len; i++) {
            TrieNode* curr = root;
            
            for (int j = i; j < len; j++) {
                int idx = name[j] - 'a';
                
                if (!curr->children[idx]) {
                    curr->children[idx] = new TrieNode();
                }
                
                curr = curr->children[idx];
                curr->strings.insert(stringId);
            }
        }
    }
    
    void dfs(TrieNode* node, int depth) {
        if (!node) return;
        
        // 如果当前节点只被一个字符串访问过
        if (node->strings.size() == 1) {
            int stringId = *node->strings.begin();
            ans[stringId]++;
        }
        
        // 递归处理子节点
        for (int i = 0; i < 26; i++) {
            if (node->children[i]) {
                dfs(node->children[i], depth + 1);
            }
        }
    }
    
public:
    void solve() {
        cin >> n;
        names.resize(n);
        ans.resize(n, 0);
        root = new TrieNode();
        
        for (int i = 0; i < n; i++) {
            cin >> names[i];
        }
        
        // 将所有字符串的所有后缀插入字典树
        for (int i = 0; i < n; i++) {
            insertAllSuffixes(i);
        }
        
        // DFS计算答案
        dfs(root, 0);
        
        for (int i = 0; i < n; i++) {
            cout << ans[i] << "\n";
        }
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    Solution solution;
    solution.solve();
    
    return 0;
}
