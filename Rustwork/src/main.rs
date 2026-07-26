use std::io;

fn main() {
    let person_one = "Sarthak";
    let person_two = "Sagar";
    let number: i64 = 9864513715;
    let number_two: i64 = 9877166372;
    let is_blocked = true;

    println!("Current caller : {}, number : {}", person_one, number);
    println!("Enter number : ");

    let mut wanna_call = String::new();
    io::stdin()
        .read_line(&mut wanna_call)
        .expect("Error, enter the correct value");

    let yes_call: i64 = wanna_call
        .trim()
        .parse()
        .expect("Error, enter the correct values");

    // Clean standard `if` check instead of `match` on boolean
    if yes_call != number_two {
        println!("Error , wrong number");
        return;
    }

    // Direct match on `is_blocked`
    match is_blocked {
        false => println!("{} is calling {}", person_one, person_two),
        true => println!(
            "Error! => You are blocked from calling {} number : {}",
            person_two, number_two
        ),
    }
}