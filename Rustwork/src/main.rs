use std::fmt::{self, Display, Formatter};
use std::ops::{Add, Div, Index, Mul, Neg, Sub};

// =============================================================================
// 1. TYPE-SAFE N-DIMENSIONAL VECTOR (Const Generics + Zero-Cost)
// =============================================================================

/// A stack-only, fixed-size vector with compile-time dimension checking.
/// Adding a Vec3 to a Vec4 is a *compile error*, not a runtime panic.
#[derive(Debug, Clone, Copy, PartialEq)]
#[repr(C)] // FFI-friendly layout, no extra padding surprises
pub struct Vector<T: Copy, const N: usize> {
    data: [T; N],
}

impl<T: Copy, const N: usize> Vector<T, N> {
    #[inline(always)]
    pub const fn new(data: [T; N]) -> Self {
        Self { data }
    }

    #[inline(always)]
    pub const fn splat(value: T) -> Self {
        Self { data: [value; N] }
    }

    #[inline(always)]
    pub const fn len(&self) -> usize {
        N
    }

    #[inline(always)]
    pub const fn as_slice(&self) -> &[T] {
        &self.data
    }
}

impl<T: Copy, const N: usize> Index<usize> for Vector<T, N> {
    type Output = T;
    #[inline(always)]
    fn index(&self, i: usize) -> &T {
        &self.data[i]
    }
}

// --- Operator Overloads (all inlined, no heap, no dynamic dispatch) ---

impl<T: Copy + Add<Output = T>, const N: usize> Add for Vector<T, N> {
    type Output = Self;
    #[inline(always)]
    fn add(mut self, rhs: Self) -> Self {
        for i in 0..N {
            self.data[i] = self.data[i] + rhs.data[i];
        }
        self
        }
}

impl<T: Copy + Sub<Output = T>, const N: usize> Sub for Vector<T, N> {
    type Output = Self;
    #[inline(always)]
    fn sub(mut self, rhs: Self) -> Self {
        for i in 0..N {
            self.data[i] = self.data[i] - rhs.data[i];
        }
        self
    }
}

impl<T: Copy + Mul<Output = T>, const N: usize> Mul<T> for Vector<T, N> {
    type Output = Self;
    #[inline(always)]
    fn mul(mut self, s: T) -> Self {
        for i in 0..N {
            self.data[i] = self.data[i] * s;
        }
        self
    }
}

impl<T: Copy + Div<Output = T>, const N: usize> Div<T> for Vector<T, N> {
    type Output = Self;
    #[inline(always)]
    fn div(mut self, s: T) -> Self {
        for i in 0..N {
            self.data[i] = self.data[i] / s;
        }
        self
    }
}

impl<T: Copy + Neg<Output = T>, const N: usize> Neg for Vector<T, N> {
    type Output = Self;
    #[inline(always)]
    fn neg(mut self) -> Self {
        for i in 0..N {
            self.data[i] = -self.data[i];
        }
        self
    }
}

impl<T: Copy, const N: usize> From<[T; N]> for Vector<T, N> {
    #[inline(always)]
    fn from(arr: [T; N]) -> Self {
        Self::new(arr)
    }
}

// --- Generic math available for ALL dimensions ---

impl<T: Copy + Mul<Output = T> + Add<Output = T>, const N: usize> Vector<T, N> {
    /// Dot product with fused accumulation.
    #[inline(always)]
    pub fn dot(self, rhs: Self) -> T {
        let mut sum = self.data[0] * rhs.data[0];
        for i in 1..N {
            sum = sum + self.data[i] * rhs.data[i];
        }
        sum
    }
}

// --- Dimension-SPECIALIZED methods ---
// This impl block ONLY exists for Vector<T, 3>. Try calling .cross() on a Vec2
// and the compiler will legitimately tell you the method doesn't exist.

impl<T: Copy + Mul<Output = T> + Sub<Output = T>> Vector<T, 3> {
    /// 3D cross product. Unavailable for any other N at compile time.
    #[inline(always)]
    pub fn cross(self, rhs: Self) -> Self {
        Self::new([
            self[1] * rhs[2] - self[2] * rhs[1],
            self[2] * rhs[0] - self[0] * rhs[2],
            self[0] * rhs[1] - self[1] * rhs[0],
        ])
    }
}

// --- f64-only conveniences ---

impl<const N: usize> Vector<f64, N> {
    #[inline(always)]
    pub fn magnitude(self) -> f64 {
        self.dot(self).sqrt()
    }

    #[inline(always)]
    pub fn normalize(self) -> Self {
        self / self.magnitude()
    }

    #[inline(always)]
    pub fn lerp(self, rhs: Self, t: f64) -> Self {
        self * (1.0 - t) + rhs * t
    }
}

impl<T: Copy + Display, const N: usize> Display for Vector<T, N> {
    fn fmt(&self, f: &mut Formatter<'_>) -> fmt::Result {
        write!(f, "[")?;
        for i in 0..N {
            if i > 0 {
                write!(f, ", ")?;
            }
            write!(f, "{}", self.data[i])?;
        }
        write!(f, "]")
    }
}

// =============================================================================
// 2. MATRIX with COMPILE-TIME DIMENSIONAL ANALYSIS
// =============================================================================

/// M×N matrix. Multiplying `Matrix<T, M, N>` by `Matrix<T, N, P>` yields
/// `Matrix<T, M, P>` — the inner dimensions MUST match or it won't compile.
#[derive(Debug, Clone, Copy, PartialEq)]
pub struct Matrix<T: Copy, const ROWS: usize, const COLS: usize> {
    data: [[T; COLS]; ROWS],
}

impl<T: Copy + Default, const R: usize, const C: usize> Matrix<T, R, C> {
    #[inline(always)]
    pub fn zero() -> Self {
        Self {
            data: [[T::default(); C]; R],
        }
    }
}

/// Type-safe matrix multiplication. The compiler verifies the "inner" dimensions
/// match at the type level: you cannot accidentally multiply a 2×3 by a 4×5.
impl<T, const M: usize, const N: usize, const P: usize> Mul<Matrix<T, N, P>> for Matrix<T, M, N>
where
    T: Copy + Mul<Output = T> + Add<Output = T> + Default,
{
    type Output = Matrix<T, M, P>;

    #[inline(always)]
    fn mul(self, rhs: Matrix<T, N, P>) -> Self::Output {
        let mut out = Matrix::zero();
        for i in 0..M {
            for j in 0..P {
                let mut sum = T::default();
                for k in 0..N {
                    sum = sum + self.data[i][k] * rhs.data[k][j];
                }
                out.data[i][j] = sum;
            }
        }
        out
    }
}

// =============================================================================
// 3. MINI RAY TRACER — renders ASCII art spheres
// =============================================================================

struct Ray {
    origin: Vector<f64, 3>,
    direction: Vector<f64, 3>,
}

impl Ray {
    fn at(&self, t: f64) -> Vector<f64, 3> {
        self.origin + self.direction * t
    }
}

struct Sphere {
    center: Vector<f64, 3>,
    radius: f64,
    color: char,
}

impl Sphere {
    /// Returns the nearest intersection distance along the ray, if any.
    fn intersect(&self, ray: &Ray) -> Option<f64> {
        let oc = ray.origin - self.center;
        let a = ray.direction.dot(ray.direction);
        let half_b = oc.dot(ray.direction);
        let c = oc.dot(oc) - self.radius * self.radius;
        let discriminant = half_b * half_b - a * c;

        if discriminant < 0.0 {
            return None;
        }

        let sqrtd = discriminant.sqrt();
        let mut root = (-half_b - sqrtd) / a;
        if root < 0.001 {
            root = (-half_b + sqrtd) / a;
            if root < 0.001 {
                return None;
            }
        }
        Some(root)
    }
}

// =============================================================================
// 4. MAIN — Demo & ASCII Render
// =============================================================================

fn main() {
    println!("=== Advanced Rust Const-Generic Playground ===\n");

    // -- Vector math demo --
    let a = Vector::new([1.0, 2.0, 3.0]);
    let b = Vector::new([4.0, 5.0, 6.0]);
    println!("a        = {}", a);
    println!("b        = {}", b);
    println!("a + b    = {}", a + b);
    println!("a · b    = {}", a.dot(b));
    println!("a × b    = {}", a.cross(b));
    println!("|a|      = {:.4}", a.magnitude());
    println!("normalize(a) = {}", a.normalize());

    // -- Matrix demo --
    // Uncomment the next line to see a beautiful compile-time error:
    // let _bad = Matrix::<f64, 2, 3>::zero() * Matrix::<f64, 4, 5>::zero();

    let m1 = Matrix::<f64, 2, 3>::zero();
    let m2 = Matrix::<f64, 3, 4>::zero();
    let m3 = m1 * m2; // 2×4 result — types check out!
    println!("\nMatrix: 2×3 * 3×4 = 2×4  ✓  (try mismatching them!)");

    // -- ASCII Ray Tracer --
    println!("\nRendering scene...\n");

    let spheres = [
        Sphere {
            center: Vector::new([0.0, 0.0, -3.0]),
            radius: 1.0,
            color: '@',
        },
        Sphere {
            center: Vector::new([1.4, -0.4, -3.8]),
            radius: 0.7,
            color: '#',
        },
        Sphere {
            center: Vector::new([-1.2, 0.2, -3.2]),
            radius: 0.5,
            color: '*',
        },
    ];

    let light = Vector::new([1.0, 1.5, -1.0]).normalize();
    let width = 64;
    let height = 32;

    for y in 0..height {
        for x in 0..width {
            // Map pixel to camera plane
            let aspect = width as f64 / height as f64;
            let u = ((x as f64 / width as f64) * 2.0 - 1.0) * aspect;
            let v = 1.0 - (y as f64 / height as f64) * 2.0;

            let ray = Ray {
                origin: Vector::new([0.0, 0.0, 0.0]),
                direction: Vector::new([u, v, -1.0]).normalize(),
            };

            let mut pixel = ' ';
            let mut closest = f64::INFINITY;

            for s in &spheres {
                if let Some(t) = s.intersect(&ray) {
                    if t < closest {
                        closest = t;
                        let hit = ray.at(t);
                        let normal = (hit - s.center).normalize();
                        let intensity = normal.dot(light).max(0.0);

                        // Simple ASCII shading ramp
                        let ramp = ['·', ':', '-', '=', '+', '*', '#', '%', '@'];
                        let idx = (intensity * (ramp.len() - 1) as f64).round() as usize;
                        pixel = ramp[idx.clamp(0, ramp.len() - 1)];
                    }
                }
            }
            print!("{}", pixel);
        }
        println!();
    }
}
