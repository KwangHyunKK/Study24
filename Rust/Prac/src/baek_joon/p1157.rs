// use std::{io, iter::Enumerate};

// pub fn run(){
//     let mut input = String::new();
//     io::stdin().read_line(&mut input).expect("No Input");
//     let input = input.trim().to_lowercase().chars().collect;
//     let mut alphabets = [0; 26];

//     for i in input.iter(){
//         let val = (input as i32 - 'a' as i32) as usize;
//         alphabets[val] = alphabets[val] + 1; 
//     }

//     let mut check = false; // multi
//     let mut index = 0; 
//     let mut maxValue = 0;

//     for (i, num) in alphabets.iter().enumerate(){
//         if *num == maxValue && check != false{
//             check = true;
//         }
//         if *num > maxValue{
//             check = false;
//             maxValue = *num;
//             index = i;
//         }
//     }

//     if check{
//         println!("?");
//     }else{
//         println!("{}", ('A' as u8 + index as u8) as char);
//     }

// }