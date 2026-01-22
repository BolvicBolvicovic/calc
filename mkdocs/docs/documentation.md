# Documentation

A calculator for my journey as an embedded system engineer.

## Data Types

There are three data types: floats, integers and complex.
When both integers and floats are used in an expression, all integers are casted to a float.

At the moment, complex are only returned when solving cubic and quartic equations. They cannot be declared and cannot be used with any math operators.

There is no intention of extending the type system concerning the data itself.
I do not want to build structures as data-layouts in the interpreter as it is a tool for math and not for programming.

However, the physics unit system enforces some kind of meta-type upon variables for certain operations.
But it is a finite set of meta-types.

## Simple expressions

**Available math operators**

- plus operator `+`
- minus operator `-`
- divide operator `/`
- multiply operator `*`
- power operator `^`

**With intergers only**

```
calc> -5 + 5 * 8 / (5 ^ 6)
```

**With at least one float value**

```
// Note: Result is float.
calc> -5 + 5.5 * 8 / (5 ^ 6)
```

**Whitespaces do not matter**

```
// Note: This is correct
calc> -5 + 5.5 * 8 / (5 ^ 6)

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

OOMs can wrap any expression.

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

### Binding operator `::`

Variables are bound to values with the binding opereator `::`.

```
calc> x :: milli(watt(5))
> 5 milli watt
calc> x + 5
> 10 milli watt
```

Note that the binding operator has the smallest precedence and can be used on both ways.

```
// This is also a valid syntax
calc> milli(watt(5)) :: x
> 5 milli watt
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
> Error while evaluating expression
```

Note that lists are internally linked lists. Accessing an element will be always O(n).

### List operations *(WIP)*

// TODO

## Math Built-ins

### Polynomials

#### `polynom_one(a, b)`

`polynom_one` takes the two first elements of a list and solves the linear equation $ax + b = 0$.
The returned value is a float.
The smallest order of magnitude is chosen between `a` and `b`.
The unit is undefined.

#### `polynom_two(a, b, c)`

`polynom_two` takes the three first elements of a list and solves the quadratic equation $ax^2 + bx + c = 0$.
The returned value is a list with the 2 roots as floats.
The smallest order of magnitude is chosen between `a`, `b` and `c`.
The unit of the roots is undefined.

#### `polynom_three(a, b, c, d)`

`polynom_three` takes the four first elements of a list and solves the cubic equation $ax^3 + bx^2 + cx + d = 0$.
The returned value is a list with the 3 roots as floats and/or complex.
The order of magnitude is the base magnitude.
The unit of the roots is undefined.

#### `polynom_four(a, b, c, d, e)`

`polynom_four` takes the five first elements of a list and solves the quartic equation $ax^4 + bx^3 + cx^2 + dx + e = 0$.
The returned value is a list with the 4 roots as floats and/or complex.
The order of magnitude is the base magnitude.
The unit of the roots is undefined.

## Other Built-Ins

### `exit`

Quits the calculator.

### `clear`

Clears the terminal.

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
