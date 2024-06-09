use std::char::ToUppercase;
use std::collections::HashMap;
use std::io;

pub fn run(){
    step1();
}

fn step1(){
    println!("문자열 입력!");

    let mut input = String::new();
    io::stdin().read_line(&mut input).expect("입력 오류");

    let char_vec: Vec<char> = input.trim().chars().collect();

    let mut char_count = HashMap::new();

    for &ch in &char_vec{
        let counter = char_count.entry(ch.to_ascii_uppercase()).or_insert(0);
        *counter += 1;
    }

    let mut index = 'A';
    let mut cnt = 0;
    let mut check = false;

    for (ch, count) in &char_count{
        if cnt < *count{
            check = false;
            cnt = *count;
            index = *ch;
        }
        else if cnt == *count{
            check = true;
        }
    }

    if check == false{
        println!("{}", index.to_ascii_uppercase());
    }else{
        println!("?");
    }
}

fn step2(){
    let mut number1 = 0;
    let mut str = String::new();
    io::stdin().read_line(&mut str).expect("No data");
    
}