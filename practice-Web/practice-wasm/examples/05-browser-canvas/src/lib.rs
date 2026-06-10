use wasm_bindgen::prelude::*;

#[wasm_bindgen]
pub fn fill_gradient(width: u32, height: u32) -> Vec<u8> {
    let mut pixels = vec![0; (width * height * 4) as usize];

    for y in 0..height {
        for x in 0..width {
            let i = ((y * width + x) * 4) as usize;
            pixels[i] = (x * 255 / width.max(1)) as u8;
            pixels[i + 1] = (y * 255 / height.max(1)) as u8;
            pixels[i + 2] = 180;
            pixels[i + 3] = 255;
        }
    }

    pixels
}

