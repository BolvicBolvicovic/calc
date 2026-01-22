# Latest Notes

Latest notes on the development state.

## Features

List of all features that I thought about and that have been validated or not, in current dev or finished.

### Ideas *(unordered)*

- comments
- autocomplete
- dot product function
- function declaration and usage
- plot function
- binary logic
- use fixed number instead of float
- make constant part of metadata OR use as a float/int
- handle squared and square roots of units
- improve user's error system
- variables assignment: `x := 5`
- lists dimensions (i.e. dim1: `(5, 6, 4, 5)`, dim2: `dim((5, 5, 8, 6), 2)`)
- equations with the `=` operator
- list operations
- helper with short documentation
- new_session helper that clears the variables map

### Validated

- square root
- complex numbers operators & declaration

### Refused

- built-in print: no need since interpreter prints results. If a variable needs to be printed, just enter the variable and it will be printed. (Variables are not implemented yet).

### In Development

- complex numbers
- built-ins:  linear, quadratic, cubic and quartic polynomials

### Finished

- parenthesis: `( expr )`
- plus operator: `expr + expr`
- minus operator: `expr - expr`
- product operator: `expr * expr`
- division operator: `expr / expr`
- pow operator: `expr ^ expr`
- implicit type system (float / int)
- identifiers
- units
- orders of magnetude
- constants: `PI`, `E`
- variables binding operator: `literal_expr :: expr`
- lists with the `,` operator

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
