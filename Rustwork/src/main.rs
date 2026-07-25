//for trapezoid method
fn function(x: f64) -> f64 {
    (4.0 * x.exp()) / (1.0 + x.powf(3.0))
}

fn trapezoid_rule(a: f64, b: f64, n: i32) -> f64 {
    let h = (b - a) / n as f64;
    let mut sum = function(a) + function(b);
    for i in 1..n {
        let x = a + i as f64 * h;
        sum += function(x);
    }
    h * sum
}

// for simpson 1/3 method
fn another_function(x: f64) -> f64 {
    x.tan() // change it to whatever you wish, i'll test with tan
}

fn simpson_rule(b: f64, a: f64, n: i32) -> f64 {
    let h = (b - a) / n as f64;
    let mut sum = another_function(a) + another_function(b);
    for i in 1..n {
        let x = a + i as f64 * h;
        if i % 2 == 0 {
            sum += 2.0 * another_function(x);
        } else {
            sum += 4.0 * another_function(x);
        }
    }
    return (h / 3.0) * sum;
}

fn main() {
    let mut a = 0.0;
    let mut b = 2.0;
    let n = [10, 20, 45, 77, 100];

    for i in 0..n.len() {
        let result = trapezoid_rule(a, b, n[i]);
        println!(
            "At n : {}, Numerical integration rule : {:.2}",
            n[i], result
        );
    }

    a = 1.0;
    b = 3.0;
    for i in 0..n.len() {
        // we can use result again as it is inside a loop.
        let result = simpson_rule(b, a, n[i]);
        println!("At n : {}, The simpson 1/3 answer is : {:.2}", n[i], result);
    }
}
