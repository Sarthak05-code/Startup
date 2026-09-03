#[allow(dead_code , non_snake_case)]
enum Status {
    Pending,
    Accpeted,
    Rejected,
}
fn Name_caller(x : i32) -> i32 {
    0
}

fn main() {
    let status = Status::Pending;

    match status {
        Status::Accpeted => println!("Your package was accepted"),
        Status::Pending => println!("Your package is pending"),
        Status::Rejected => println!("Your package was rejeted"),
    }
}
