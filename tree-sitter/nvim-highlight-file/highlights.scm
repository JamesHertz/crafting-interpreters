[ "for"  "while"  ] @keyword.repeat
[ "if"   "else"   ] @keyword.condition
[ "and"  "or"     ] @keyword.operator
[ "true" "false"  ] @boolean
[ "(" ")" "{" "}" ] @punctuation.bracket
[ "+" "-" "*" "/" ">=" "<=" ">" "<" "==" "!" "=" ] @operator
";"      @punctuation.delimiter
"return" @keyword.return
"var"    @keyword
"fun"    @keyword.function
"print"  @function.builtin


(identifier) @variable
(number)     @number
(string)     @string

(function_arguments (identifier) @variable.parameter)
(func_def fun_name: (identifier) @function)
