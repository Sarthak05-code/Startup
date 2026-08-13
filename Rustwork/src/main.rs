use std::sync::atomic::{AtomicI32, Ordering};

static COUNTER: AtomicI32 = AtomicI32::new(0);

fn name_caller(name: &str) -> i32 {
    let count = COUNTER.fetch_add(1, Ordering::SeqCst) + 1;
    println!("Hello {}", name);
    count
}

fn main() {
    let name = String::from("Sarthak");

    let count = name_caller(&name);
    let count = name_caller(&name);
    let count = name_caller(&name);

    println!("The number in count : {count}");
}
