package jh.craft.interpreter.representation;

public class AstPrinter implements Expr.Visitor<String>{
    public String print(Expr exp){
        return exp.accept( this );
    }

    @Override
    public String visitBinary(Expr.Binary binary) {
        var left = binary.left().accept( this );
        var right = binary.right().accept( this );
        var operator = binary.operator();

        return String.format("(%s %s %s)", operator.lexeme(), left, right);

    }

    @Override
    public String visitLiteral(Expr.Literal literal) {
        var value = literal.value();
        return value instanceof String
                ? String.format("'%s'", value) : value.toString();
    }

    @Override
    public String visitGrouping(Expr.Grouping group) {
        var exp = group.expression().accept( this );
        return String.format("(group %s)", exp);
    }

    @Override
    public String visitUnary(Expr.Unary unary) {
        var op = unary.operator();
        var exp = unary.expression().accept(this);
        return String.format("%s(%s)", op.lexeme(), exp);
    }

    @Override
    public String visitVariable(Expr.Variable variable) {
        return null;
    }

    @Override
    public String visitAssign(Expr.Assign assign) {
        return null;
    }

    @Override
    public String visitLogical(Expr.Logical logical) {
        return null;
    }
}
