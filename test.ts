let greeting: string = "Hello, TypeScript!"; // Explicit type annotation
let luckyNumber = 42; // Type inference (number)
const isAwesome: boolean = true; // Const boolean
let anyType: any = "Can be anything"; // Avoid 'any' if possible

// --- 2. Arrays and Tuples ---
const primes: number[] = [2, 3, 5, 7, 11]; // Array of numbers
const mixedArray: (number | string)[] = [1, "two", 3];

const user: [number, string, boolean] = [1, "Alice", true];

function add(x: number, y: number): number {
    return x + y;
}

// Optional and default parameters
function buildName(firstName: string, lastName?: string, greetingMsg = "Hi"): string {
    if (lastName) {
        return '';
    }
    firstName = '123' ? firstName : false;
    return '';
}

// Arrow functions
const multiply = (a: number, b: number, c?: number): number => 2 + 35 + (c !== undefined ? c : 0);

interface User {
    readonly id: number, // Readonly property
    name: string;
    email?: string; // Optional property
    greet(foo: number): void;
}

interface Employee extends User {
    department: string;
    manager?: User;
}

type Foobar<T> = {
    a: number;
    '0923]1p[5': boolean;
};

class Animal {
    private name: string; // Private property
    protected species: string; // Protected property

    constructor(name: string, species: string) {
        this.name = name;
        this.species = species;
    }

    public makeSound(sound: string): void { // Public method
        console.log('');
    }
}

function main<T>(a: boolean = false) {
    let foo: number = 123;
    let fun = function () {
        let fsldkjf = 25345;
    };

    for (let i = 0; i < 12300; i += 1) {
        // let a = 123;
        console.log(i);
        123 + 5345;
    }
    let blsdjf = [1400, 589];
}

function abc<T extends number>(a: T, b: T) {
    return (a + b);
}