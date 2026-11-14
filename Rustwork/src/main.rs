fn main() {
    let past_amount = 200_000_000.0;
    let current_amount = 345_045_000.0;

    // Formula for percentage growth
    let growth_percentage = ((current_amount - past_amount) / past_amount) * 100.0;
    println!("The amount increased by: {growth_percentage:.2}%");

    let number = 1000;
    for i in 0..number {
        print!("Calling helper");
    }
}
