use std::{fmt::Display, ops::Rem};

fn return_number(number: i32) -> Result<i32, String> {
    if number % 2 == 0 {
        return Err(String::from("Even Number Not allowed"));
    }

    let answer = number / 2;
    Ok(answer)
}

fn runner(value: i32) -> Result<(), String> {
    let final_answer = return_number(value)?;

    println!("The answer is : {final_answer}");

    Ok(())
}

fn return_paragraph(text: &str) -> Result<String, String> {
    if !text.starts_with('*') || !text.ends_with('*') {
        return Err(String::from("The value is not formated correctly"));
    }

    let formatted = text.trim_matches('*');
    Ok(format!("<p>{formatted}</p>"))
}

fn paragraph_runner(text: &str) -> Result<(), String> {
    let result = return_paragraph(text)?;
    println!("{result}");

    Ok(())
}

fn main() {
    if let Err(err) = runner(33) {
        println!("Error : {err}")
    }

    if let Err(err) = paragraph_runner("*Hello, World") {
        println!("Error : {err}")
    }

    print_max_value("Highest Score", 10, 20);
    print_max_value("Highest Score in Percenatage", 10.0, 20.0);
    print_max_value("Alphabetic Order", "Sarthak", "Sagar");
    print_max_value("Boolean Order", true, false);
    let answer = even_or_odd(10);
    println!("Answer : {answer}");

    if is_vowel('a') {
        println!("Yes , a is an vowel.")
    } else {
        println!("No, not an vowel")
    }
}

fn is_vowel(char: char) -> bool {
    let vowels = ['a', 'e', 'i', 'o', 'u'];
    for v in vowels {
        if char == v {
            return true;
        }
    }
    false
}

fn print_max_value<T>(label: &str, a: T, b: T)
where
    T: PartialOrd + Display,
{
    if a > b {
        println!("{label} : {a}");
    } else {
        println!("{label} : {b}");
    }


}

fn even_or_odd<T>(a: T) -> String
where
    T: Rem<Output = T> + PartialEq + From<u8> + Copy,
{
    let zero = T::from(0u8);
    let two = T::from(2u8);

    if a % two == zero {
        String::from("Even")
    } else {
        String::from("Odd")
    }
}
