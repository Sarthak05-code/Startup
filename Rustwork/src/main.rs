fn main() {
    // 1. Initialize a mutable vector to act as a stack
    let mut stack = Vec::new();

    // 2. Push elements onto the stack
    stack.push(10);
    stack.push(20);
    stack.push(30);

    println!("Stack after pushes: {:?}", stack); // [10, 20, 30]

    // 3. Pop elements off the stack (returns Option<T>)
    if let Some(top) = stack.pop() {
        println!("Popped: {}", top); // 30
    }

    println!("Stack after pop: {:?}", stack); // [10, 20]
}
