use std::io;

pub fn run(){
    println!("Hello, World!");

    run1();
}

fn run1(){
    let val: Vec<i32> = vec![5,7,11];

    println!("{:?}", val);

    let n: i64 = 2222222 * 2222222;
    println!("{}", n);
}


fn run2(){
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();

    let input = input.trim();

    let parts: Vec<&str> = input.split('=').collect();

    if parts.len() != 2{
        println!("False input. You shall input '=' ");
        return;
    }

    let left_expr = parts[0].trim();
    let right_result: i32 = match parts[1].trim().parse(){
        Ok(val) => val,
        Err(_) => {
            println!("It is not valid number");
            return;
        }
    };

    let operators = ['+', '-', '*', '/'];
    let operator_pos = left_expr.find(&operators[..]);

    if let Some(pos) = operator_pos{
        let operator = left_expr.chars().nth(pos).unwrap();
        let (left_operand, right_operand) = left_expr.split_at(pos);
        let left_operand: i32 = match left_operand.trim().parse(){
            Ok(val) => val,
            Err(_) =>{
                println!("left operand is not valid number");
                return;
            }
        };

        let right_operand: i32 = match right_operand[1..].trim().parse(){
            Ok(val) => val,
            Err(_) => {
                println!("right operand is not valid number");
                return;
            }
        };

        let result = match operator {
            '+' => left_operand + right_operand,
            '-' => left_operand - right_operand,
            '*' => left_operand * right_operand,
            '/' => {
                if right_operand == 0 {
                    println!("0으로 나눌 수 없습니다.");
                    return;
                }
                left_operand / right_operand
            }
            _ => {
                println!("지원되지 않는 연산자입니다.");
                return;
            }
        };

        if result == right_result{
            println!("Correct value!");
        }else{
            println!("Incorrect value!");
        }
    }else{
        println!("No operand in expression.");
    }
}



