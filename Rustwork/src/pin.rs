use std::f64::consts::PI;

struct Circle {
    radius: f64,
}

struct Rectangle {
    length: f64,
    breadth: f64,
}

impl Circle {
    fn new(radius: f64) -> Self {
        Circle { radius }
    }
}

trait PerimeterArea {
    fn area(&self) -> f64;
    fn perimeter(&self) -> f64;
}

impl PerimeterArea for Circle {
    fn area(&self) -> f64 {
        PI * self.radius * self.radius
    }

    fn perimeter(&self) -> f64 {
        2.0 * PI * self.radius
    }
}

impl PerimeterArea for Rectangle {
    fn area(&self) -> f64 {
        self.length * self.breadth
    }

    fn perimeter(&self) -> f64 {
        2.0 * (self.breadth + self.length)
    }
}

fn main() {
    let circle = Circle::new(12.0);
    let rectangle = Rectangle {
        length: 14.0,
        breadth: 19.0,
    };

    println!(
        "{:.2} is the circle area, {:.2} is the circle perimeter",
        circle.area(),
        circle.perimeter()
    );
    println!(
        "{:.2} is the rectangle area, {:.2} is the rectangle perimeter",
        rectangle.area(),
        rectangle.perimeter()
    );
}
