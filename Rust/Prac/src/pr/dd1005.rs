use std::collections::HashMap;

// A method of counting the frequency of the input string and arranging it in descending order based on the frequency.
pub fn run(){
    let input = String::from("Hello, 1005");

    let mut frequency : HashMap<char, u32> = HashMap::new();

    for c in input.chars(){
        if c.is_whitespace(){
            continue;
        }
        *frequency.entry(c).or_insert(0) += 1;
    }

    let mut frequency_vec: Vec<(&char, &u32)> = frequency.iter().collect();
    frequency_vec.sort_by(|a, b| b.1.cmp(a.1));

    for (ch, count) in frequency_vec{
        println!("'{}' : {}", ch, count);
    }
}