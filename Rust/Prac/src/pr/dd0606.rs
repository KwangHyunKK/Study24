use std::io;

pub fn run(){
    let mut input = String::new();
    println!("Enter two nubmers separated by space : ");

    io::stdin().read_line(&mut input).expect("Failed to read line");

    let parts: Vec<&str> = input.trim().split_whitespace().collect();

    if parts.len() != 2{
        println!("Please enter exactly two numbers separated by space.");
        return;
    }

    let a:i32 = parts[0].parse().expect("Not a valid number");
    let b:i32 = parts[1].parse().expect("Not a valida number");
    match a{
        _=> println!("The number is {}", a)
    }

    let sum = a + b;

    println!("The sum of {} and {} is {}",a, b, sum);

}