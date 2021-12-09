# Tests

These tests override the random number generator to produce the same rolls every time. This is done by passing an additional `double` to `Lexer_Lex` with a value of anything except 0. 

Unfortunately, the random number generator used (`std::mt19937`) is implementation specific and as such, fixing the seed in this way will produce different results depending on the compiler you are using. 

The 'correct' values used by these tests were obtained using `g++ 8.1.0` and as such they will fail if a different compiler is used. 