use std::io;

fn run() {
    // 사용자로부터 입력 받기
    println!("몇 번째 피보나치 수를 구할까요?");
    
    let mut input = String::new();
    io::stdin().read_line(&mut input).expect("입력을 읽는 중 오류 발생");
    let n: usize = input.trim().parse().expect("숫자를 입력해야 합니다");

    // DP를 위한 배열 초기화
    let mut dp = vec![0; n + 1];

    // 초기 조건
    if n >= 1 {
        dp[1] = 1;
    }

    // DP를 이용해 피보나치 수 계산
    for i in 2..=n {
        dp[i] = dp[i - 1] + dp[i - 2];
    }

    // 결과 출력
    println!("{} 번째 피보나치 수는: {}", n, dp[n]);
}
