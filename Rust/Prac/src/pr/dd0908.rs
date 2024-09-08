use std::rc::{Rc, Weak};
use std::cell::RefCell;
use std::io::{self, Write};

struct Node {
    value: i32,
    parent: RefCell<Weak<Node>>, // 부모에 대한 약한 참조
    children: RefCell<Vec<Rc<Node>>>, // 자식에 대한 강한 참조
}

pub fn run() {
    let parent = Rc::new(Node {
        value: 5,
        parent: RefCell::new(Weak::new()),
        children: RefCell::new(vec![]),
    });

    let child = Rc::new(Node {
        value: 10,
        parent: RefCell::new(Rc::downgrade(&parent)),
        children: RefCell::new(vec![]),
    });

    parent.children.borrow_mut().push(Rc::clone(&child));

    println!("Parent value: {}", parent.value);
    println!("Child's parent value: {}", child.parent.borrow().upgrade().unwrap().value);
}

trait Person{
    fn name(&self) -> &str;
    fn introduce(&self){
        println!("안녕하세요, 저는 {} 입니다.", self.name());
    }
}

struct Student{
    name: String,
    grade: u8,
}

impl Person for Student{
    fn name(&self)-> &str{
        &self.name
    }
}

impl Student{
    fn study(&self){
        println!("{} 학생의 공부가 시작되었습니다.", self.name);
    }

    fn new() -> Student{
        let mut name = String::new();
        let mut grade = String::new();

        print!("학생의 이름 : ");
        io::stdout().flush().unwrap();
        io::stdin().read_line(&mut name).unwrap();
        let name = name.trim().to_string();

        print!("학생의 학년 입력 : ");
        io::stdout().flush().unwrap();
        io::stdin().read_line(&mut grade).unwrap();
        let grade = grade.trim().parse().expect("올바른 숫자 입력!");

        Student {name, grade}
    }
}

struct Teacher{
    name : String,
    subject : String,
}

impl Person for Teacher{
    fn name(&self) -> &str{
        &self.name
    }
}

impl Teacher{
    fn teach(&self){
        println!("{} 선생님이 {} 수업을 가르칩니다.", self.name, self.subject);
    }
}

struct Classroom{
    name: String,
    teacher: Teacher,
    students: Vec<Student>,
}

impl Classroom{
    fn class_start(&self){
        println!("{} 교실에서 수업 시작.", self.name);
        self.teacher.teach();
        for student in &self.students{
            student.study();
        }
    }
}

struct School{
    name: String,
    classrooms : Vec<Classroom>,
}

impl School{
    fn start_day(&self){
        println!("{} 에서 새로운 하루 시작", self.name);
        for classroom in &self.classrooms{
            classroom.class_start();
        }
    }
}

fn run1(){
    // 학생 2명 입력받기
    let student1 = Student::new();
    let student2 = Student::new();

    // 선생님 생성
    let teacher = Teacher {
        name: String::from("김선생"),
        subject: String::from("수학"),
    };

    // 교실 생성
    let classroom = Classroom {
        name: String::from("3학년 A반"),
        teacher: teacher,
        students: vec![student1, student2],
    };

    // 학교 생성
    let school = School {
        name: String::from("대한민국 초등학교"),
        classrooms: vec![classroom],
    };

    // 학교에서 하루를 시작
    school.start_day();
}