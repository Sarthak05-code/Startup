// Math functions to integrate
fn f_trapezoid(x: f64) -> f64 {
    (4.0 * x.exp()) / (1.0 + x.powf(3.0))
}

fn f_simpson(x: f64) -> f64 {
    x.tan()
}


fn trapezoid_rule(f: fn(f64) -> f64, a: f64, b: f64, n: usize) -> f64 {
    let h = (b - a) / n as f64;
    let mut sum = f(a) + f(b);

    for i in 1..n {
        let x = a + i as f64 * h;
        sum += 2.0 * f(x); 
    }

    (h / 2.0) * sum
}

// Fixed Simpson's 1/3 Rule
fn simpson_rule(f: fn(f64) -> f64, a: f64, b: f64, n: usize) -> Result<f64, &'static str> {
    if n % 2 != 0 {
        return Err("Simpson's 1/3 rule requires n to be even.");
    }

    let h = (b - a) / n as f64;
    let mut sum = f(a) + f(b);

    for i in 1..n {
        let x = a + i as f64 * h;
        if i % 2 == 0 {
            sum += 2.0 * f(x);
        } else {
            sum += 4.0 * f(x);
        }
    }

    Ok((h / 3.0) * sum)
}

fn main() {
    // --- Trapezoidal Rule ---
    let a_trap = 0.0;
    let b_trap = 2.0;
    let n_trap = [10, 20, 45, 77, 100];

    println!("--- Trapezoidal Rule ---");
    for &n in &n_trap {
        let result = trapezoid_rule(f_trapezoid, a_trap, b_trap, n);
        println!("At n = {:3}: Result = {:.6}", n, result);
    }

    // --- Simpson's 1/3 Rule ---
    let a_simp = 1.0;
    let b_simp = 3.0;
    // Note: n must be even for Simpson's 1/3 Rule
    let n_simp = [10, 20, 46, 78, 100]; 

    println!("\n--- Simpson's 1/3 Rule ---");
    for &n in &n_simp {
        match simpson_rule(f_simpson, a_simp, b_simp, n) {
            Ok(result) => println!("At n = {:3}: Result = {:.6}", n, result),
            Err(err) => println!("At n = {:3}: Error -> {}", n, err),
        }
    }
}