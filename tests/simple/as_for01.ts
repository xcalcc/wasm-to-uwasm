declare function sayHello(): void;

sayHello();

export let q:i32;

export function add(x: i32, y: i32): i32 {
    for (let i:i32 = 0; i < x; i++) {
        q = q * y + i;
    }
    return x / y;
}
