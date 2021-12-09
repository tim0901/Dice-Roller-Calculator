import colorama
import dice_roller_calculator as roller

from colorama import Fore
from colorama import Style
colorama.init()

r = roller.Roller()

number_of_passes = 0
number_of_fails = 0
number_of_tests= 0

def test(test_string, expected_return_string, expected_total):
    text,total = r.Roll_Test(test_string)
    if(total % 1 == 0):
        print(text + " = " + str(int(total)))
    else:
        print(text + " = " + str(total))
    global number_of_passes
    global number_of_fails
    global number_of_tests
    number_of_tests += 1
    if(text == expected_return_string and total == expected_total):    
        print(f'{Fore.GREEN}PASS{Style.RESET_ALL}')
        number_of_passes += 1
    else:
        print(f'{Fore.RED}FAIL{Style.RESET_ALL}')
        number_of_fails += 1


# Test roll
test("1d20", "1d20 (9)", 9)

### Testing advantage/disadvantage
test("1d20 + 5 adv", "2d20kh1 (~~9~~, **20**) + 5", 25)
test("1d20 + 5 dis", "2d20kl1 (9, ~~**20**~~) + 5", 14)

## Testing KH/KL etc
test("5d20kh3", "5d20kh3 (~~9~~, **20**, 15, 19, ~~**1**~~)", 54)
test("5d20kl3", "5d20kl3 (9, ~~**20**~~, 15, ~~19~~, **1**)", 25)
test("5d20dh3", "5d20kl2 (9, ~~**20**~~, ~~15~~, ~~19~~, **1**)", 10)
test("5d20dl3", "5d20kh2 (~~9~~, **20**, ~~15~~, 19, ~~**1**~~)", 39)

# This should parse to 2d20 + 5 - can't have advantage on multi-die rolls
test("2d20 + 5 adv", "2d20 (9, **20**) + 5", 34)
    
### Testing min/max
test("10d20min2", "10d20min2 (9, **20**, 15, 19, ~~**1**~~ -> 2, 3, 7, **20**, 3, 5)", 103)
test("10d20max2", "10d20max2 (~~9~~ -> 2, ~~**20**~~ -> 2, ~~15~~ -> 2, ~~19~~ -> 2, **1**, ~~3~~ -> 2, ~~7~~ -> 2, ~~**20**~~ -> 2, ~~3~~ -> 2, ~~5~~ -> 2)", 19)

## Testing rerolls
test("1d20r9", "1d20r9 (~~9~~ -> **20**)", 20)
test("10d20r<9", "10d20r<9 (9, **20**, 15, 19, ~~**1**~~ -> 2, ~~3~~ -> 8, ~~7~~ -> 4, **20**, ~~3~~ -> 8, ~~5~~ -> 7)", 112)
test("10d20r>9", "10d20r>9 (9, ~~**20**~~ -> 2, ~~15~~ -> 8, ~~19~~ -> 4, **1**, 3, 7, ~~**20**~~ -> 8, 3, 5)", 50)

## Testing multiple rolls
test("2d20 + 5d6", "2d20 (9, **20**) + 5d6 (5, **6**, **1**, **1**, 2)", 44)
    
## Testing nested rolls
test("(1d20)d(1d20)", "(1d20 (9))d(1d20 (**20**)) (15, 19, **1**, 3, 7, **20**, 3, 5, 2)", 75)

## Testing mathematical functions

## Addition
test("123+456", "123 + 456", 579)

## Subtraction
test("123-456", "123 - 456", -333)
    
## Negative numbers
test("-456 + 123", "- 456 + 123", -333)

## Multiplication
test("123*456", "123 * 456", 56088)
    
## Division
test("5/2", "5 / 2", 2.5)

## Indices
test("2^ (4 + 5)", "2^(4 + 5)", 512)

# Brackets
test("2(3+(4*5))(6+7)^2", "2 * (3 + (4 * 5)) * (6 + 7)^2", 7774)

# Combination
test("2(2d20kl1 + 2(4d6min5)) + (3d8)^(2+1) + 7", "2 * (2d20kl1 (**1**, ~~3~~) + 2 * (4d6min5 (~~3~~ -> 5, **6**, 5, **6**))) + (3d8 (3, **8**, 2))^(2 + 1) + 7", 2294)

print(str(number_of_passes) + " passes and " + str(number_of_fails) + " fails out of " + str(number_of_tests) + " tests.")

if(number_of_fails > 0):
    print("The results for these tests were obtained using the GCC/G++ compiler. Other compilers will produce different results that will not pass these tests.")