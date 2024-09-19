use std::io;

pub fn run(){
    let mut input = String::new();
    io::stdin().read_line(&mut input).unwrap();
    let mut arr : Vec<i32> = input.trim().split_ascii_whitespace().map(|s| s.parse().expect("error")).collect();

    println!("변환된 숫자들 : {:?}", arr);

    arr.sort();

    println!("정렬된 숫자들 : {:?}", arr);

    println!("Lower_bound nubmers");
    let mut target_input = String::new();
    io::stdin()
        .read_line(&mut target_input)
        .expect("No input");
    let target: i32 = target_input.trim().parse().expect("Can't transform");

    let target: i32 = target_input.trim().parse().expect("숫자로 변환할 수 없습니다.");

    // lower_bound 함수 호출
    let lb_index = lower_bound(&arr, target);

    // 결과 출력
    if lb_index < arr.len() {
        println!("lower_bound 인덱스: {}, 값: {}", lb_index, arr[lb_index]);
    } else {
        println!("lower_bound 값이 없습니다.");
    }
}

fn lower_bound(arr: &[i32], target: i32) -> usize {
    let mut low = 0;
    let mut high = arr.len();

    while low < high{
        let mid = (low + high) / 2;
        if arr[mid] < target{
            low = mid + 1;
        }else{
            high = mid;
        }
    }
    low
}