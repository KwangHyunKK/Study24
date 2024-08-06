// 트럭 문제
use std::io;
use std::collections::VecDeque;

pub fn run(){
    solve();
}

fn input() -> Vec<i16>{
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();
    let mut output : Vec<i16> = Vec::new();
    output = match input.trim().split_whitespace().map(|s| s.parse()).collect(){
        Ok(num) => num,
        Err(num) => Vec::new(),
    };
    input.clear();
    io::stdin().read_line(&mut input).unwrap();
    let mut sub_output : Vec<i16> = Vec::new();
    sub_output = match input.trim().split_whitespace().map(|s| s.parse()).collect(){
        Ok(num) => num,
        Err(num) => Vec::new(),
    };

    output.extend(sub_output.clone());

    output
}

fn solve(){
    let inputs = input();

    // Input Check passed
    let mut sum = 0; // 무게 합
    let mut count : i16 = 1; // count * inputs[1] = answer
    let mut queue : VecDeque<i16> = VecDeque::new();
    let value = inputs[2]; // bridge weight
    let mut idx = 3;
    loop{
        count = count + 1;

        while queue.len() == 0 || sum + inputs[idx] < value || idx < inputs.len(){
            queue.push_back(count);
            sum = sum + inputs[idx];
            idx = idx + 1;    
        }

        loop{
            if let Some(v) = queue.front(){
                if v == &count{
                    queue.pop_front();
                    sum = sum - 
                }                
            }else{
                break;
            }
        }
    }

    println!("count : {}", count);
}

// timer count 세고
// 배열에 시간 넣고
// 시간 되면 queue.front() 해서 빼고
// queue.empty 인 시간을 출력하는 코드