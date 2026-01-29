# Latest Notes

Latest notes on the development state.

## Features

List of all features that I thought about and that have been validated or not, in current dev or finished.

### Ideas *(unordered)*

#### Math & Physics

- dot product function
- binary logic
- plot function
- make constant part of metadata OR use as a float/int
- handle unkown conversion (i.e. watt/second)

#### Language Design

- function declaration and usage
- variables assignment: `x := 5`
- equations with the `=` operator
- list operations
- lists dimensions (i.e. dim1: `(5, 6, 4, 5)`, dim2: `dim((5, 5, 8, 6), 2)`)

#### User Interface

- comments
- autocomplete
- helper with short documentation
- new_session helper that clears the variables map

### Validated

*NONE*

### Refused

- built-in print: no need since interpreter prints results. If a variable needs to be printed, just enter the variable and it will be printed.
- use fixed number instead of float: slows down computation since it needs to work with complex numbers and unnecessary precision in a physics context.

### In Development

- improve user's error system: handle syntax error

### Finished

- parenthesis: `( expr )`
- plus operator: `expr + expr`
- minus operator: `expr - expr`
- product operator: `expr * expr`
- division operator: `expr / expr`
- pow operator: `expr ^ expr`
- implicit type system (float / complex)
- identifiers
- units: volt, ampere, watt, ohm, joule, second
- orders of magnetude
- constants: `PI`, `E`, `I`
- variables binding operator: `literal_expr :: expr`
- lists with the `,` operator
- maths built-ins: polynomials, trigo and cube/square roots
- physics built-ins: current related calculators
- practical built-ins: exit, clear
- errors as enums

### Spotted issues

*NONE*

## Lexer

- **Operators**: `+`, `-`, `/`, `^`, `::`, `*`
- **Delimiters**: precedence `(` and `)`,
- **floats**: with/without a dot (i.e. `2.5`, `-9`)
- **identifier**: string containg isalpha(char) or `_` character

## Pratt Parser

- **identifier**: same as lexer's
- **constant value**: written floats and integers
- **unary operator**: operator that operates based on a left expression at the moment, just substraction (i.e `-7`)
- **binary operator**: operator that operates on both a right expression and a left expression (i.e `8 + 7`)
