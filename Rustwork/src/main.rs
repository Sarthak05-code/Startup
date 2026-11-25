#![allow(unused)]

#[allow(non_camel_case_types)]
struct BaseStats;

impl BaseStats {
    const BASE_SPEED: f64 = 107.0;
    const SPEED_THRESHOLD: f64 = 140.0;
    const BASE_ELATION_PERCENTAGE: f64 = 0.0;
    const ELATION_PERCENTAGE: f64 = 40.0;
    const MAX_ELATION_PERCENTAGE: f64 = 100.0;
    const MAX_ELATION_TO_SPEED_LIMIT: f64 =
        BaseStats::MAX_ELATION_PERCENTAGE - BaseStats::ELATION_PERCENTAGE;
}

fn main() {
    // Try changing this to 140.0, 145.0, or 250.0 to test the math!
    let mut speed = 145.0;
    let mut elation_percentage = BaseStats::BASE_ELATION_PERCENTAGE;

    println!("The current speed: {speed:.2} and base elation(not calculated): {elation_percentage:.2}");

    // Using >= handles both the exact threshold and values beyond it safely
    if speed >= BaseStats::SPEED_THRESHOLD {
        if speed == BaseStats::SPEED_THRESHOLD {
            println!("Bonus damage percentage at exact `140` activated.");
        }

        // Calculate how much speed exceeded the 140.0 threshold
        let extra_speed = speed - BaseStats::SPEED_THRESHOLD;

        // Caps the elation percentage safely to MAX_ELATION_PERCENTAGE (100.0)
        if extra_speed > BaseStats::MAX_ELATION_TO_SPEED_LIMIT {
            elation_percentage = BaseStats::MAX_ELATION_PERCENTAGE;
        } else {
            elation_percentage = BaseStats::ELATION_PERCENTAGE + extra_speed;
        }
    }

    println!("Final speed : {speed:.2}");
    println!("Final elation : {elation_percentage:.2}");
}
