# Dice Roller / Calculator
 A C++-based dynamic library for rolling dice - perfect for tabletop RPGs.


# Dice Roller
The dice roller uses standard inline dice rolling conventions for commands such as 

```
1d20 + 5
```

or 

```
3d8 + 2d6 + 1
```

and supports a range of modifiers:

```
kh: keep highest
kl: keep lowest
dh: drop highest
dl: drop lowest
r: re-roll a given result (eg re-roll 1s)
r<: re-roll results below a threshold value
r>: re-roll results above a threshold value
min: replace rolls below a threshold value
max: replace rolls above a threshold value
adv: advantage - acts as kh1. Only works for single-die rolls.
dis: disadvantage, acts as kl1. Only works for single-die rolls.
```

You can also roll nested rolls, such as:

```
(1d20)d(1d20) = (1d20 (9))d(1d20 (**20**)) (15, 19, **1**, 3, 7, **20**, 3, 5, 2) = 75
```

# Calculator

The library also acts as a basic calculator, supporting addition/subtraction, multiplication/division, indices and brackets while maintaining operator priority. This allows for relatively complex expressions to be evaluated:

```
2(2d20kl1 + 2(4d6min5)) + (3d8)^(2+1) + 7 = 2 * (2d20kl1 (**1**, ~~3~~) + 2 * (4d6min5 (~~3~~ -> 5, **6**, 5, **6**))) + (3d8 (3, **8**, 2))^(2 + 1) + 7 = 2294
```

# Building

I recommend building with the included Makefile, which uses g++. 

# Usage

I recommend looking at the examples folder for examples of how to hook into the library. 

To process commands, you must first create a `Lexer` object through `Lexer_new()`. Commands are then processed by passing the following parameters to `Lexer_Lex':

```
the Lexer object you created
the expression to be evaluated as a c string (const char[])
a pre-allocated c string (char[]) buffer for the output
an unsigned integer containing the size of the above buffer in bytes
a pre-allocated pointer to a double (double*) through which the final total shall be returned
```

which returns a c string containing the parsed expression, modified to include the rolled values for each die. Markdown is used to **bold** the maximum and minimum values for rolls, as well as to ~~strikethrough~~ rejected rolls. See above for examples of what this looks like.
