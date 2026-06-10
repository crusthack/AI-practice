use wasm_bindgen::prelude::*;

#[wasm_bindgen]
pub fn add(left: i32, right: i32) -> i32 {
    left + right
}

#[wasm_bindgen]
pub fn classify_score(score: i32) -> String {
    match score {
        90..=100 => "excellent",
        70..=89 => "solid",
        0..=69 => "needs-practice",
        _ => "invalid",
    }
    .to_string()
}

