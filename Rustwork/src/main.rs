fn main() {
    let mut number = 1;
    {
        number += 1;
        println!("Inside the  {number}");
    }
    number += 1;
    println!("Outside : {number}");
}
