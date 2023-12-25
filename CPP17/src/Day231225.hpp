#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <locale>

int Makefile(const char* input, const char* output){
    // 파일의 폴더의 위치와 이름은 변수를 생성할 때 생성자의 인수로 제공
    // 파일의 오픈은 객체의 생성과 함께 이루어진다.
    // 파일이 존재하지 않더라도 에러나 예외가 발생하지 않는다.
    std::ifstream input_file(input);
    if(!input_file.is_open()){
        std::cout << input << "파일을 열 수 없습니다.\n";
        return 1;
    }

    std::ofstream output_file(output);
    if(!output_file.is_open()){
        std::cout << output << "파일을 열 수 없습니다.\n";
        return 1;
    }

    char data[100];

    while(input_file >> data){
        output_file << data;
    }

    // 데이터를 모두 읽었으면 닫는다.
    input_file.close();
    output_file.close();
    return 0;
}

int ReadFile(const char* input){
    std::ifstream input_file(input);
    if(!input_file.is_open()){
        std::cout << input << "파일을 열 수 없습니다.\n";
        return 1;
    }

    std::string data;

    // 파일의 데이터를 읽어 출력
    while(input_file >> data)std::cout << data << "\n";

    input_file.close();
    return 0;
}

int run1(){
    const char* file = "output.txt";
    
}