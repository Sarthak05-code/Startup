use std::f64::consts::PI;

trait HasArea {
    fn area(&self) -> f64;
}

struct Circle {
    radius: f64,
}

struct Rectangle {
    width: f64,
    height: f64,
}

impl Circle {
    fn new(radius: f64) -> Self {
        Circle { radius }
    }
}

impl HasArea for Circle {
    fn area(&self) -> f64 {
        PI * self.radius * self.radius
    }
}

impl HasArea for Rectangle {
    fn area(&self) -> f64 {
        self.width * self.height
    }
}

fn main() {
    let circle = Circle::new(5.0);
    let rectangle = Rectangle {
        width: 5.0,
        height: 10.0,
    };
    println!("{:.2} is the area of circle. ", circle.area());
    println!("{} is the area of rectangle. ", rectangle.area());
}
