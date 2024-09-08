use std::rc::{Rc, Weak};
use std::cell::RefCell;

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
