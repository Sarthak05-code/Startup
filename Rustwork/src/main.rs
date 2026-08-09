#![allow(warnings)]
use std::f64::consts::PI;

struct Shapes {
    radius: f64,
    length: f64,
    breadth: f64,
}
trait Areas {
    fn area(&self) -> f64;
}

impl Areas for Shapes {
    fn area(&self) -> f64 {
        PI * self.radius * self.radius
    }
}

fn main() {
    let circle = Shapes {
        radius: 12.0,
        length: 12.0,
        breadth: 12.0,
    };
    println!("{:.2} is the area", circle.area());
}
