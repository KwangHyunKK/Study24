use std::io;

pub fn run(){
    println!("Let't move acmicpc_1037!");
    acmicpc_1037();
}

fn acmicpc_1037(){
    // 양수 A가 N의 진짜 약수가 모두 주어지면, N을 구하는 프로그램
    let mut input = String::new();

    io::stdin().read_line(&mut input).expect("No data!");
    let parts : Vec<&str> = input.trim().split_ascii_whitespace().collect();

    if parts.len() == 1{
        let num = parts[0].parse().unwrap_or(0);
        let ans = num * num;
        println!("answer is : {}", ans);
    }else{
        let mut max = 0;
        let mut min = 1000001;
        for i in 0..parts.len(){
            let num = parts[i].parse().unwrap_or(0);
            if max < num{
                max = num;
            }

            if min > num{
                min = num;
            }
        }
        println!("answer is : {}", min * max);
    }
}
