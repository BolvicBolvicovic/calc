# Latest Notes

Latest notes on the development state.

## Features

List of all features that I thought about and that have been validated or not, in current dev or finished.

### Ideas *(unordered)*

#### Math

- dot product function
- binary logic
- plot function
- make constant part of metadata OR use as a float/int

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
- improve user's error system

### Validated

*NONE*

### Refused

- built-in print: no need since interpreter prints results. If a variable needs to be printed, just enter the variable and it will be printed. (Variables are not implemented yet).
- use fixed number instead of float: slows down computation since it needs to work with complex numbers.

### In Development

- adding more physics units
- resistance addition in series / parallel

### Finished

- parenthesis: `( expr )`
- plus operator: `expr + expr`
- minus operator: `expr - expr`
- product operator: `expr * expr`
- division operator: `expr / expr`
- pow operator: `expr ^ expr`
- implicit type system (float / complex)
- identifiers
- units
- orders of magnetude
- constants: `PI`, `E`, `I`
- variables binding operator: `literal_expr :: expr`
- lists with the `,` operator
- square/cube root
- built-ins:  polynomials, trigo and cube/square roots
- practical built-ins: exit, clear

### Spotted issues

*NONE*

## Lexer

- **Operators**: `+`, `-`, `/`, `^`, `::`, `*`
- **Delimiters**: precedence `(` and `)`,
- **floats**: with a dot (i.e. `2.5`)
- **int**: without a dot (i.e. `5`)
- **identifier**: string containg isalpha(char)

## Pratt Parser

- **identifier**: same as lexer's
- **constant value**: written floats and integers
- **unary operator**: operator that operates based on a left expression at the moment, just substraction (i.e `-7`)
- **binary operator**: operator that operates on both a right expression and a left expression (i.e `8 + 7`)
