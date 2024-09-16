use std::collections::HashMap;

// Trie 노드를 정의합니다.
#[derive(Debug, Default)]
struct TrieNode {
    children: HashMap<char, TrieNode>,
    is_end_of_word: bool,
}

#[derive(Debug, Default)]
struct Trie {
    root: TrieNode,
}

impl Trie {
    // 새로운 Trie를 생성합니다.
    fn new() -> Self {
        Trie {
            root: TrieNode::default(),
        }
    }

    // 단어를 트라이에 삽입합니다.
    fn insert(&mut self, word: &str) {
        let mut node = &mut self.root;
        for ch in word.chars() {
            node = node.children.entry(ch).or_insert_with(TrieNode::default);
        }
        node.is_end_of_word = true;
    }

    // 트라이에서 단어가 존재하는지 확인합니다.
    fn search(&self, word: &str) -> bool {
        let mut node = &self.root;
        for ch in word.chars() {
            if let Some(next_node) = node.children.get(&ch) {
                node = next_node;
            } else {
                return false;
            }
        }
        node.is_end_of_word
    }

    // 주어진 prefix로 시작하는 단어가 있는지 확인합니다.
    fn starts_with(&self, prefix: &str) -> bool {
        let mut node = &self.root;
        for ch in prefix.chars() {
            if let Some(next_node) = node.children.get(&ch) {
                node = next_node;
            } else {
                return false;
            }
        }
        true
    }
}

pub fn run() {
    let mut trie = Trie::new();

    // 단어 삽입
    trie.insert("apple");
    trie.insert("app");
    trie.insert("banana");

    // 검색 테스트
    println!("Search for 'apple': {}", trie.search("apple")); // true
    println!("Search for 'app': {}", trie.search("app"));     // true
    println!("Search for 'ban': {}", trie.search("ban"));     // false
    println!("Starts with 'app': {}", trie.starts_with("app")); // true
    println!("Starts with 'ban': {}", trie.starts_with("ban")); // true
    println!("Starts with 'abc': {}", trie.starts_with("abc")); // false
}
