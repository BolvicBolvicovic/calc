# Documentation

A calculator for my journey as an embedded system engineer.

## Data Types

There are two data types: float and complex.
Type cast precedence: float < complex.

Complex numbers can be computed with the `I` value.

There is no intention of extending the type system concerning the data itself.
I do not want to build structures as data-layouts in the interpreter as it is a tool for math/physics and not for programming.

However, the physics unit system enforces some kind of meta-type upon variables for certain operations.
But it is a finite set of meta-types.

## Simple expressions

**Available math operators**

- plus operator `+`
- minus operator `-`
- divide operator `/`
- multiply operator `*`
- power operator `^`

**A basic expression**

```
// Note: Result is float.
calc> -5 + 5.5 * 8 / (5 ^ 6)
```

**Whitespaces do not matter**

```
// Note: This is correct
calc>    -5+ 5.5 * 8 / (5 ^ 6)

// Note: This too
calc> -5+5.5*8/(5^6)
```

**Result is printed**

```
calc> -5 + 5 * 8 / (5 ^ 6)
> -5
```

## Physics Units

**Definition**

Physics units are subtypes that cannot affect a variable's value. They are metadata about a variable.
However, they can interact with each other, reducing their expression to the most consise form.
Example: **ampere * volt = watt**

**Available built-in units**

- volt
- ampere
- ohm
- watt
- seconds
- joule

**Usage**

Physics units can wrap any expression that do not have a defined physics unit yet.

```
calc> ampere(5) * volt(5)
> 25 watt
```

If a unit conversion is not supported, the calculus is still done but a clear message signals the issue.

```
calc> ampere(5) * watt(5)
> 25 unsupported unit
```

The only exception to the main rule is when making an operation between two expressions
that have physics units but only one of them has a defined order of magnitude.
In that case, the expression without an order of magnitude is considered to be of magnitude 1 (baseline).

```
calc> milli(volt((5)) + volt(5)
> 5005 milli volt
```

## Orders of Magnitude

**Definition**

Orders of magnitude (OOMs) are subtypes that can affect a variable's value. They are also metadata about a variable.

**Available built-in magnitudes**

- base_magnitude
- deci
- centi
- milli
- micro
- nano

**Usage**

OOMs can wrap any expression that does not result in a complex number.
Giving an OOM to a complex number will not do anything to it.

```
calc> milli(ampere(5)) * milli(volt(5))
> 0.025 milli watt
```

When OOMs meet each other, they default to the smallest.

```
calc> milli(ampere(5)) * volt(5)
> 25 milli watt
```

When one of the two expressions does not have an OOM, it is automatically converted to the OOM of the second expression.

```
calc> milli(ampere(5)) * 5
> 25 milli ampere
```

If an expression has already an OOM, wrapping it with a new OOM updates the variable and its OOM accordingly.
It means that OOMs do not enforce any kind of type since the conversion is automatically done.

```
calc> base_magnitude(milli(ampere(5)) * volt(5))
> 0.025 watt
```

## Variables

Variables are identifiers that are bound to a value.
The name *variable* is to be taken lightly as variations can only happen between different sessions as states of the interpreter are not saved.
However, its bound value is constant and it cannot be rebound to another value once it has been bound.

Note: I have given some thought about an assignment operator which is classic in programming
but since it is a calculator, I do not think that assignment is as meaningfull.
We are not looking for performance on the frontend, just some math.
However, I have not refused the idea and I am still thinking about it.

### Binding operators `::` and `:`

Global variables are bound to values with the global binding opereator `::`.

```
calc> x :: milli(watt(5))
> 5 milli watt
calc> x + 5
> 10 milli watt
```

Note that the global binding operator has the smallest precedence and can be used on both ways.

```
// This is also a valid syntax
calc> milli(watt(5)) :: x
> 5 milli watt
```

Temporary variables are bound to values with the temporary binding opereator `:`.
They share their lifetime (or scope) with the expression.

```
calc> (c : 5) + c
> 10
calc> c * c
> Error at line 1 here ->* c: Operation with unbound variable not allowed
```

## Lists

The list is the container of the calc interpreter. A list is a list of values (floats or integer).
At the moment, they can only be used as arguments for built-ins.

Later on, they could be used to declare matrices, to be returned as result of equations and more...

### List operator `,`

A list is defined with the `,` operator as `(left_expr, right_expr)`.

```
calc> my_list :: (12, 5, 6)
> (12, 5, 6)
```

One can access an element of a list with a 0-based index like so:

```
calc> my_list(0)
> 12
calc> my_list(10)
> Error at line 1 here ->10): Out of bound
```

Note that lists are internally linked lists. Accessing an element will be always O(n).

## Function

### Declaration

A function is declared with the built-in `func`. The expression in its parenthesis is saved and can be used later.

```
calc> circle :: func(2 * PI * _R)
> ((2)*(PI))*(_R)
```

### Usage

A function call works in a similar manner a built-in works.
A list of temporary defined values with the temporary binding operator `:` is passed to the function.

```
calc> circle(_R:5)
> 31.4159
```

Note that if you pass any other argument to the function, (i.e. a single value like 5), the function execution fails because one of the variable is undefined.

```
calc> circle(5)
> Error at line 1 here ->*: Operation with unbound variable not allowed 
```

Here `_R` is undefined. Note that variables can be defined on a global scope. A placeholder needs to be passed to the function list to make the call.

```
calc> circle :: func(2 * PI * _R)
> ((2)*(PI))*(_R)
calc> _R :: 5
> 5
calc> circle(_)
> 31.4159
```

## Math Built-ins

### Polynomials

Note for each polynomial that the numbers in the list are considered as floats.
If a complex number is passed to one of the function, it is *undefined behaviour*.
The reason for this is that the user knows that these values are supposed to be Real and not Complex.
If this turns out to be a problem, extensive checks will be added.
The order of magnitude for each root is the base magnitude and the unit is undefined.

#### `polynom_one(a, b)`

`polynom_one` takes the two first elements of a list and solves the linear equation $ax + b = 0$.
The returned value is a float.

#### `polynom_two(a, b, c)`

`polynom_two` takes the three first elements of a list and solves the quadratic equation $ax^2 + bx + c = 0$.
The returned value is a list with the 2 roots as floats.

#### `polynom_three(a, b, c, d)`

`polynom_three` takes the four first elements of a list and solves the cubic equation $ax^3 + bx^2 + cx + d = 0$.
The returned value is a list with the 3 roots as floats and/or complex.

#### `polynom_four(a, b, c, d, e)`

`polynom_four` takes the five first elements of a list and solves the quartic equation $ax^4 + bx^3 + cx^2 + dx + e = 0$.
The returned value is a list with the 4 roots as floats and/or complex.

### Trigonometry

Handles complex numbers.

- `cos(a)`
- `arccos(a)`
- `tan(a)`
- `arctan(a)`
- `sin(a)`
- `arcsin(a)`

### Others

#### `sqrt(a)`

Square root of a number. Returns a float or a complex number.

#### `cbrt(a)`

Cube root of a number. Returns a float or a complex number.

### Constants

*(WIP) Maybe make constant part of metadata AND/OR use as a float/int.*

Constants are known values that are associated with literals in capital letters.

`PI` *(π)*

```
calc> PI
> 3.14159
```

`E` *(Euler's number)*

```
calc> E
> 2.71828
```

`I` *(I² = -1)*

```
calc> I*I
> -1
```

## Physics Built-Ins

All physics built-ins take floats as parameters.
Special cases that work with complex numbers are stated in the related documentation.

### Electricity

#### `current(a,b)`

Calculates all parameters missing parameters of the current based the two known parameters.
`a` and `b` cannot have the same physics unit and must be either of unit `ohm`, `ampere`, `watt` or `volt`.
The retunred value is a list containing the two missing parameters.
On error, returns an error.

```
calc> current(volt(5), ampere(5))
> (25 watt, 1 ohm)
calc> current(watt(5), ohm(5))
> (1 ampere, 5 volt)
```

#### `res_parallel(r1, r2, ..., rn)`

Calculates the resistance for resistances that are in parallel in a circuit.
The unit of each value is considered as `ohm` and is not checked.
The return value is a single float of unit `ohm`.

```
calc> res_parallel(15, 87, 20)
> 7.80269 ohm
```

#### `volt_divider(v, r1, r2, ..., rn)`

Calculates the voltage drop for each resistances in series in a circuit.
The unit of the first argument is considered as `volt`. 
The unit of each other argument is considered as `ohm`.
The return value is a list of floats of unit `volt` matching the resistances.

```
calc> volt_divider(15, 100, 50)
> (10 volt, 5 volt)
```

#### `amp_divider(a, r1, r2, ..., rn)`

Calculates the amps drop for each resistances in parallel in a circuit.
The unit of the first argument is considered as `ampere`. 
The unit of each other argument is considered as `ohm`.
The return value is a list of floats of unit `ampere` matching the resistances.

```
calc> amp_divider(15, 100, 100)
> (7.5 ampere, 7.5 ampere)
```

## Other Built-Ins

### `exit`

Quits the calculator.

### `clear`

Clears the terminal.

### `new_session`

Clears the variables hashmap.
By extension, variables are not bound to their previous values.

### `unbind(var_name)`

Deletes a variable from the variables hashmap.
By extension, the variable can be bound to a new value.
Use in extreme cases only.
