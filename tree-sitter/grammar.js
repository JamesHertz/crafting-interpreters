/// <reference types="tree-sitter-cli/dsl" />
// @ts-check

const PREC = {
  assignment : 1,
  or         : 2,
  and        : 3,
  equality   : 4,
  comparison : 5,
  term       : 6,
  factor     : 7,
  unary      : 8,
  primary    : 9,
};

module.exports = grammar({
  name: "lox",
  extras: $ => [
    /\s/,
    $.comment,
  ],
  word: $ => $.identifier,
  rules: {
    source_file : $ => repeat($._declaration),
    _declaration : $ => choice(
      $.func_def,
      seq($.var_decl, ';'),
      $.statement,
    ),
    func_def: $ => seq('fun', field('fun_name', $.identifier), '(', optional($.function_arguments), ')', $.block),
    function_arguments: $ => seq($.identifier, repeat(seq(',', $.identifier))),
    var_decl  : $ => seq('var', $.identifier, optional(seq('=', $._expression)), ),
    statement : $ => choice(
      seq('print', $._expression, ';'),
      seq('while', '(', $._expression, ')', $.statement),
      seq('for', '(',
        optional(choice($.var_decl, $._expression)), ';', optional($._expression), ';', optional($._expression),
        ')', $.statement
      ),
      prec.right(0, seq('if', '(', $._expression, ')', $.statement, optional(seq('else', $.statement)))),
      seq('return', optional($._expression), ';'),
      seq($._expression, ';'),
      $.block,
    ),
    block       : $ => seq('{', repeat($._declaration), '}'),
    _expression : $ => choice(
      prec.left(PREC.or,  seq($._expression, 'or', $._expression)),
      prec.left(PREC.and, seq($._expression, 'and', $._expression)),

      prec.left(PREC.equality, seq($._expression, '==', $._expression)),
      prec.left(PREC.equality, seq($._expression, '!=', $._expression)),

      prec.left(PREC.comparison, seq($._expression, '>',  $._expression)),
      prec.left(PREC.comparison, seq($._expression, '<',  $._expression)),
      prec.left(PREC.comparison, seq($._expression, '>=', $._expression)),
      prec.left(PREC.comparison, seq($._expression, '<=', $._expression)),

      prec.left(PREC.factor, seq($._expression, '*', $._expression)),
      prec.left(PREC.factor, seq($._expression, '/', $._expression)),

      prec.left(PREC.term,   seq($._expression, '+', $._expression)),
      prec.left(PREC.term,   seq($._expression, '-', $._expression)),

      prec(PREC.assignment,  seq(field('assign_target', $.identifier), '=', $._expression)),
      $.unary,
      $._primary,
    ),
    unary      : $ => prec(PREC.unary, seq(choice('-', '!'), $._expression)),
    _primary   : $ => prec(PREC.primary,
        choice($.string, $.identifier, $.number, seq('(', $._expression, ')'), "true", "false", $.call)
    ),
    call       : $ => seq(field('function', $.identifier), "(", optional(seq($._expression, repeat(seq(',', $._expression)))) ,")"),
    identifier : _ => /[a-zA-Z_]+/,
    number     : _ => /[0-9]+(.[0-9]+)?/,
    string     : _ => /"[^\n"]*"/,
    comment    : _ => /\/\/[^\n]*/,
  }
});
